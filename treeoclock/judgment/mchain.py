import itertools
import sys
import os
import pandas as pd
import numpy as np
import itertools

from treeoclock.judgment import ess, _plots
from treeoclock.judgment import _geweke_diag as gwd
from treeoclock.trees.time_trees import TimeTreeSet
from treeoclock.summary.compute_sos import compute_sos_mt
from treeoclock.summary.frechet_mean import frechet_mean
from treeoclock import enums

_geweke_kind = {"default": 0, "crossed": 1, "doublecrossed": 2, "crosscompare": 3, }


class MChain:
    def __init__(self, trees, log_file, summary, working_dir):

        # Setting trees
        if type(trees) is TimeTreeSet:
            self.trees = trees
        elif type(trees) is str:
            self.trees = TimeTreeSet(trees)
        else:
            raise ValueError(trees)

        # Reading the log_file
        if os.path.exists(log_file):
            self.log_data = pd.read_csv(log_file, header=0, sep=r"\s+", comment="#")
            self.chain_length = int(list(self.log_data["Sample"])[-1])
            self.log_sampling_interval = int(list(self.log_data["Sample"])[1])
            # assuming that the first iteration 0 tree and the last have been logged in the logfile
            self.tree_sampling_interval = self.chain_length / (len(self.trees) - 1)
        # todo the logfile should be optional as it is possible to compute all new measures without the log file
        #  it is only relevant if we want to compare them or do something with those values
        #  maybe add the option logfile None which also throws errors when functions are called that might use the logfile
        else:
            if type(log_file) is str:
                raise FileNotFoundError(log_file)
            else:
                raise ValueError(log_file)

        if summary is not None:
            # Setting the summary tree
            if type(summary) is TimeTreeSet:
                self.summary = summary
            elif type(summary) is str:
                self.summary = TimeTreeSet(summary)
            else:
                raise ValueError(summary)

        if working_dir is not None:
            # Setting the Working directory
            if os.path.isdir(working_dir):
                self.working_dir = working_dir
            else:
                if type(working_dir) is str:
                    raise NotADirectoryError(working_dir)
                else:
                    raise ValueError(working_dir)

        if summary is not None:
            # Check mapping of summary tree and tree set
            if not self.summary.map == self.trees.map:
                try:
                    self.summary.change_mapping(self.trees.map)
                except ValueError as error:
                    raise ValueError(f"{error}\n"
                                     f"The given summary tree and tree set do not fit! "
                                     f"\n(Construction of class MChain failed!)")

    def get_key_names(self):
        return list(self.log_data.columns)[1:]

    def get_ess(self, ess_key="posterior", ess_method="arviz", **kwargs):
        if type(ess_method) is str:
            if not hasattr(ess, f"{ess_method}_ess"):
                raise ValueError(f"The given ESS method {ess_method} does not exist!")
        else:
            raise ValueError(ess_method)
        lower_i = 0
        if "lower_i" in kwargs:
            lower_i = kwargs["lower_i"]
        upper_i = self.log_data.shape[0] - 1
        if "upper_i" in kwargs:
            upper_i = kwargs["upper_i"]
        if "lower_i" in kwargs or "upper_i" in kwargs:
            if type(lower_i) is not int or type(upper_i) is not int:
                raise ValueError("Wrong type for upper or lower index!")
            if upper_i > self.log_data.shape[0]:
                raise IndexError(f"{upper_i} out of range!")
            if lower_i > upper_i or lower_i < 0 or upper_i < 0:
                raise ValueError("Something went wrong with the given upper and lower index!")

        if ess_key in list(self.log_data.columns):
            chain_length = 1
            if upper_i != lower_i:
                chain_length = int(self.log_data["Sample"][upper_i]) - int(self.log_data["Sample"][lower_i])
            cur_sampling_interval = int(chain_length / (
                    self.log_data[ess_key][lower_i:(upper_i + 1)].shape[0] - 1 - self.log_data[ess_key][lower_i:(
                    upper_i + 1)].isna().sum()))
            return getattr(ess, f"{ess_method}_ess")(data_list=self.log_data[ess_key][lower_i:(upper_i + 1)].dropna(),
                                                     chain_length=chain_length,
                                                     sampling_interval=cur_sampling_interval)
        else:
            raise ValueError("Not (yet) implemented!")

    # todo ideally this should be accessible with the get_ess() funciton and ess_key="pseudo"
    def get_pseudo_ess(self, **kwargs):
        # todo all the testing for upper_i and lower_i but ideally this will be part of the other funciton so this is temporary
        # todo should return the same as RWTY package but also possibly computed with the RNNI distance measure
        upper_i = len(self.trees)
        if "upper_i" in kwargs:
            upper_i = kwargs["upper_i"]
        chain_length = 1
        if upper_i > 0:
            chain_length = (self.chain_length / (len(self.trees) - 1)) * (upper_i - 1)
        return ess.pseudo_ess(tree_set=self.trees[0:upper_i], chain_length=chain_length,
                              sampling_interval=self.chain_length / (len(self.trees) - 1))

    # todo missing tests
    # the rnni variance log is essentially a running mean plot
    def compute_rnni_variance_log(self, focal_tree_type="tree", norm=False, add=True):
        new_log_list = []
        for i in range(0, len(self.trees)):
            # todo this type checking is not efficient, should be outside of the loop!
            if focal_tree_type == "FM":
                focal_tree = frechet_mean(self.trees[0:i + 1])
            elif focal_tree_type == "tree":
                focal_tree = self.trees[i]
            elif focal_tree_type == "centroid":
                sys.exit("Not yet implemented")
                focal_tree = frechet_mean(self.trees[0:i + 1])
            else:
                raise ValueError(f"Unknown type {focal_tree_type} given!")
            new_log_list.append(compute_sos_mt(focal_tree, self.trees[0:i + 1], n_cores=None, norm=norm) / (i + 1))

        # adding the computed log value to the log dataframe

        if add:
            self.add_new_log_list(new_log_list=new_log_list, col_key=f"Var{'_norm' if norm else ''}_{focal_tree_type}")
        return new_log_list

    # todo missing test and proper management of the average parameter
    def compute_new_tree_distance_log(self, average: enums.Average = enums.Average.MEAN, norm: bool = False,
                                      add: bool = True):
        new_log_list = [0]  # initialize as the first iteration is just one tree
        for i in range(1, len(self.trees)):
            new_log_list.append(
                average.function([self.trees[i].fp_distance(self.trees[j], norm=norm) for j in range(0, i)]))
        if add:
            self.add_new_log_list(new_log_list=new_log_list,
                                  col_key=f"Distance{'_norm' if norm else ''}_{average.name}")
        return new_log_list

    def compute_new_tree_summary_distance_log(self, summary="FM", norm=False, add=True):
        new_log_list = [0]  # initialize as the first iteration is just one tree
        for i in range(1, len(self.trees)):
            if summary is "FM":
                new_log_list.append(self.trees[i].fp_distance(frechet_mean(self.trees[0:i]), norm=norm))
            elif summary is "Centroid":
                raise ValueError("Not yet implemented!")
            else:
                raise ValueError(f"Not implemented summary type {summary}!")
        if add:
            self.add_new_log_list(new_log_list=new_log_list, col_key=f"Distance{'_norm' if norm else ''}_{summary}")
        return new_log_list

    # todo missing some proper tests
    def compute_mean_in_chain_deviation(self, norm=False, add=True):
        new_log_list = [1, 1]
        distance_list = {f"{r},{s}": self.trees.fp_distance(r, s, norm=norm) ** 2 for r, s in
                         list(itertools.permutations(range(len(self.trees)), 2))}
        cur_sum = 0
        seen = {}
        for i in range(2, len(self.trees)):
            for r, s in list(itertools.permutations(range(i), 2)):
                if f"{r},{s}" not in seen:
                    cur_sum += distance_list[f"{r},{s}"]
                    seen[f"{r},{s}"] = 1
            new_log_list.append(cur_sum / (i * (i - 1)))
        if add:
            self.add_new_log_list(new_log_list=new_log_list, col_key=f"In_Chain_deviation{'_norm' if norm else ''}")
        return new_log_list

    # todo missing tests
    def compute_geweke_deviation(self, norm=False, add=True):
        # todo still WIP, name should change, also parameter checking before calling the funciton

        new_log_list, intersum_list, list10, list40 = gwd.geweke_diagnostic_distances(trees=self.trees, norm=norm)

        if add:
            self.add_new_log_list(new_log_list=new_log_list, col_key=f"Geweke_deviation{'_norm' if norm else ''}")
            self.add_new_log_list(new_log_list=intersum_list, col_key=f"Intersum deviation{'_norm' if norm else ''}")
            self.add_new_log_list(new_log_list=list10, col_key=f"list10{'_norm' if norm else ''}")
            self.add_new_log_list(new_log_list=list40, col_key=f"list40{'_norm' if norm else ''}")
        return new_log_list

    # todo missing proper tests
    def compute_ess_traces(self, all=True, partial=[]):
        # computes the ess traces for all the statistic values in the data
        if (not all) and partial:
            # only compute the given list of statistics ess traces
            sys.exit("Currently Feature is not implemented!")
        ess_keys = self.get_key_names()
        method = 'tracerer'  # todo this can be either arviz, coda or tracerer

        for key in ess_keys:
            self.log_data[f"{key}_ess_cum_trace"] = np.nan
            # todo the start of this range/loop could be an argument given
            for i in range(2, self.log_data.shape[0]):
                if not np.isnan(self.log_data[key][i]):
                    self.log_data[f"{key}_ess_cum_trace"][i] = self.get_ess(ess_key=key, ess_method=method, upper_i=i)
        return 0

    # todo missing proper testing
    def compute_geweke_diag(self, summary="FM", norm=True, kind="default", add=True):

        # todo should get fraction 1 and fraction 2 as argument, default 0.1 and 0.5

        # todo this actually does not need to have a summary tree? just use a randomly fixed tree or two randomly fixed trees?
        #  compare the results

        if kind not in _geweke_kind:
            raise ValueError(f"Given geweke kind {kind} not recognized!")

        # todo export all the stuff to external funciton and file, call with _geweke_kind[kind] to specify the thing that shall be returned?
        #  here the external funciton gets an integer which corresponds to list[] which is a specific function maybe?

        new_log_list = [1]
        for i in range(1, len(self.trees)):
            if summary is "FM":
                if i < 10:  # todo temporary
                    # Setting 10 to be the smallest tree set for which the value is actually computed
                    new_log_list.append(1)
                else:
                    sec10 = self.trees[int(i * 0.1):int(i * 0.2)]
                    last40 = self.trees[int(i * 0.6):i]

                    if kind == "default":
                        var10 = compute_sos_mt(frechet_mean(sec10), sec10, norm=norm) / len(sec10)
                        var40 = compute_sos_mt(frechet_mean(last40), last40, norm=norm) / len(last40)
                        new_log_list.append(abs(var10 - var40))
                    elif kind == "crossed":
                        var10_in40 = compute_sos_mt(frechet_mean(sec10), last40, norm=norm) / len(last40)
                        var40_in10 = compute_sos_mt(frechet_mean(last40), sec10, norm=norm) / len(sec10)
                        new_log_list.append(abs(var10_in40 - var40_in10))
                    elif kind == "doublecrossed":
                        # compares the variation of two different trees for the same set
                        fm10 = frechet_mean(sec10)
                        fm40 = frechet_mean(last40)
                        var10 = compute_sos_mt(fm10, sec10, norm=norm) / len(sec10)
                        var40 = compute_sos_mt(fm40, last40, norm=norm) / len(last40)
                        var10_in40 = compute_sos_mt(fm10, last40, norm=norm) / len(last40)
                        var40_in10 = compute_sos_mt(fm40, sec10, norm=norm) / len(sec10)
                        new_log_list.append(abs(var10_in40 - var10) + abs(var40_in10 - var40))
                    elif kind == "crosscompare":
                        # compares the variation of two trees in one set
                        fm10 = frechet_mean(sec10)
                        fm40 = frechet_mean(last40)
                        var10 = compute_sos_mt(fm10, sec10, norm=norm) / len(sec10)
                        var40 = compute_sos_mt(fm40, last40, norm=norm) / len(last40)
                        var10_in40 = compute_sos_mt(fm10, last40, norm=norm) / len(last40)
                        var40_in10 = compute_sos_mt(fm40, sec10, norm=norm) / len(sec10)
                        new_log_list.append(abs(var40_in10 - var10) + abs(var10_in40 - var40))
                    # todo maybe a kind that computes the mean of all the others?
                    # todo maybe a kind that will just return all variations
                    else:
                        raise ValueError(f"The given kind {kind} is not recognized!")

            elif summary is "Centroid":
                raise ValueError("Not yet implemented!")
            else:
                raise ValueError(f"Not implemented summary type {summary}!")
        if add:
            self.add_new_log_list(new_log_list=new_log_list,
                                  col_key=f"Geweke_diag{'_norm' if norm else ''}_{kind}_{summary}")
        return new_log_list

    # todo missing tests
    # todo this is only applicable if the added list is starting at 0
    #  if it starts with index 5 the sampling interval and everything is messed up
    def add_new_log_list(self, new_log_list, col_key):
        if len(new_log_list) == self.log_data.shape[0]:
            self.log_data[col_key] = new_log_list
        elif len(new_log_list) < self.log_data.shape[0]:
            if self.tree_sampling_interval % self.log_sampling_interval == 0:
                sampling_diff = int(self.tree_sampling_interval / self.log_sampling_interval)
                self.log_data[col_key] = np.nan
                self.log_data[col_key][::sampling_diff] = new_log_list
            else:
                sys.exit("Feature not implemented! Trees logged is not a multiple of the log sampling interval!")
        elif len(new_log_list) > self.log_data.shape[0]:
            sys.exit("Feature not yet implemented! "
                     "The logfile contains less data than the tree file!")

    def get_ess_trace_plot(self, ess_key="all", ess_method="arviz", kind="cummulative"):
        # todo add kind window ?
        #  add a interval size for the plot
        # todo all the checks for the variables given
        # todo ess method can be list of all methods and then it will create multiplot thing with distribution on the diagonal

        if ess_key == "all":
            ess_key = self.get_key_names()
        elif type(ess_key) is str and ess_key in self.log_data:
            ess_key = [ess_key]
        elif not (type(ess_key) is list and all(item in self.get_key_names() for item in ess_key)):
            raise ValueError("ess_key wrong type")

        data = []
        for i in range(5, self.log_data.shape[0]):  # Starting at sample 5 as it is not useful to look at less samples
            data.extend([[key, self.get_ess(ess_key=key, ess_method=ess_method, upper_i=i), i] for key in ess_key])
        data = pd.DataFrame(data, columns=["Ess_key", "Ess_value", "Upper_i"])
        _plots._ess_trace_plot(data)
        return 0

    def get_trace_plot(self, value_key='posterior'):
        if value_key not in self.log_data:
            raise KeyError(f"Given Value {value_key} does not exist!")
        _plots._log_trace_plot(self.log_data[value_key][:5])
        return 0

    # todo add function that add the ESS traces as columns in the dataframe for the Tracer Visualization
    #  this is easier than writing my own plots

    def write_log_file(self, path):
        # todo check if path is correct
        #  throw warning if file exitsts and stuff like that

        # todo make this a function to calculate all the values and then write a tracer compatible log file
        #  this should also have a parameter accepting columns which will then be included in the log file
        #  default all columns including the original log values from beast logfile

        # todo this should take the working directory into account?

        # This should output a csv file that is compatible with Tracer to visualize all the values
        self.log_data.dropna().to_csv(path, sep="\t", index=False)

    def do_all_the_things(self, out_file, only_norm=True):
        # todo computes all the parameters possible, and then writes a logfile that is compatible with tracer

        # todo should also use parallel processes for each thing!

        # todo write things to the file whenever finished

        # todo should check if the outfile exists and if it does only add the things that are missing!

        # todo Enum for parameter setting instead of the current stuff

        if not only_norm:
            # todo compute all the un normed parameters also
            sys.exit("currently not implemented feature!")

        self.log_data.dropna().to_csv(out_file, sep="\t", index=False)
