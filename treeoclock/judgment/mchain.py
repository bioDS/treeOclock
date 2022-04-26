import pandas as pd
import os

from treeoclock.judgment import ess, _ess_plots
from treeoclock.trees.time_trees import TimeTreeSet


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
            self.log_data = _read_beast_logfile(log_file)
            self.chain_length = int(list(self.log_data["Sample"])[-1])
            self.sampling_interval = int(list(self.log_data["Sample"])[1])
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

    # todo maybe add some default values here?
    def get_ess(self, ess_key, ess_method, **kwargs):
        if type(ess_method) is str:
            if not hasattr(ess, f"{ess_method}_ess"):
                raise ValueError(f"The given ESS method {ess_method} does not exist!")
        else:
            raise ValueError(ess_method)
        lower_i = 0
        if "lower_i" in kwargs:
            lower_i = kwargs["lower_i"]
        upper_i = self.log_data.shape[0]
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
            return getattr(ess, f"{ess_method}_ess")(data_list=self.log_data[ess_key][lower_i:upper_i],
                                                     chain_length=int(self.log_data["Sample"][upper_i - 1]) -
                                                                  int(self.log_data["Sample"][lower_i]),
                                                     sampling_interval=self.sampling_interval)
        else:
            raise ValueError("Not (yet) implemented!")

    def get_ess_trace_plot(self, ess_key, ess_method, kind="cummulative"):
        # todo add kind window ?

        # todo needs a list of ess_keys and ess_method
        #  interval size defaulting to 1 so all samples

        # todo all the checks for the variables given

        data = []
        for i in range(self.log_data.shape[0] - 1):
            data.append([ess_key, self.get_ess(ess_key=ess_key, ess_method=ess_method, upper_i=i), i])

        data = pd.DataFrame(data, columns=["Ess_key", "Ess_value", "Upper_i"])
        _ess_plots._ess_trace_plot(data)

        return 0

    # todo change the name of this plot!!!!
    def get_burnin_comparison(self):
        # todo gets a focal tree defaulting to summary tree
        #  if that is None, either pick one of the trees or compute something with options here
        return 0


def _read_beast_logfile(logfile_path):
    data = pd.read_csv(logfile_path, header=0, sep=r"\s+", comment="#", error_bad_lines=True)
    return data
