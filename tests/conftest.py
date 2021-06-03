import pytest

from testfixtures import TempDirectory
from treeoclock.trees.time_trees import TimeTreeSet

@pytest.fixture()
def dir():
    with TempDirectory() as dir:
        yield dir


@pytest.fixture
def five_taxa_newick_list():
    # list of pairwise distances is [0, 1, 2, 1, 0, 3, 2, 3, 0]
    return ["((1:3,5:3):1,(4:2,(3:1,2:1):1):2);", "((1:2,5:2):3,(4:3,(3:1,2:1):2):2);",
            "((1:2,5:2):3,(3:3,(4:1,2:1):2):2);"]


@pytest.fixture
def five_taxa_list_distances():
    return [0, 1, 2, 1, 0, 3, 2, 3, 0]


@pytest.fixture
def five_taxa_tts(dir, five_taxa_nexus_string):
    dir.write("test.nex", five_taxa_nexus_string)
    return TimeTreeSet(f'{dir.path}/test.nex')


@pytest.fixture
def five_taxa_nexus_string():
    return ("#NEXUS\n"
            "BEGIN TAXA;\n"
            "\tDIMENSIONS NTAX = 5;\n"
            "\tTAXLABELS\n"
            "\t\tt1\n"
            "\t\tt5\n"
            "\t\tt4\n"
            "\t\tt3\n"
            "\t\tt2\n"
            "\t;\n"
            "END;\n"
            "BEGIN TREES;\n"
            "\tTRANSLATE\n"
            "\t1	t1,\n"
            "\t2	t5,\n"
            "\t3	t4,\n"
            "\t4	t3,\n"
            "\t5	t2\n"
            "\t;\n"
            "\tTREE * t1 = [&R] ((1:3,2:3):1,(3:2,(4:1,5:1):1):2);\n"
            "\tTREE * t2 = [&R] ((1:2,2:2):3,(3:3,(4:1,5:1):2):2);\n"
            "\tTREE * t3 = [&R] ((1:2,2:2):3,(4:3,(3:1,5:1):2):2);\n"
            "END;\n").encode()


@pytest.fixture
def seventeen_taxa_tree_newick():
    return "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5," \
           "((12:13,(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16); "


@pytest.fixture
def seventeen_taxa_rank_neighbours_newick():
    return [
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:4,(1:2,(7:1,2:1):1):2):2):1,(13:3,14:3):4):3):5,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:4,3:4):2,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:5,14:5):2):3):5,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):2,(13:4,14:4):4):2):5,((12:13,"
        "(17:7,16:7):6):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:8,4:8):2,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,"
        "(17:9,16:9):4):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):2,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):4):4,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:10,10:10):2):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((9:13,"
        "(11:11,10:11):2):1,(12:12,(17:8,16:8):4):2):1):1,15:16);"
    ]


@pytest.fixture
def five_taxa_0_nni_neighbours():
    return [
        "((1:3,5:3):1,(3:2,(4:1,2:1):1):2);",
        "((1:3,5:3):1,(2:2,(3:1,4:1):1):2);",
        "((1:3,(4:2,(3:1,2:1):1):1):1,5:4);",
        "(((4:2,(3:1,2:1):1):1,5:3):1,1:4);"
    ]


@pytest.fixture
def five_taxa_0_all_neighbours():
    return [
        "((1:3,5:3):1,(3:2,(4:1,2:1):1):2);",
        "((1:3,5:3):1,(2:2,(3:1,4:1):1):2);",
        "(((4:2,(3:1,2:1):1):1,5:3):1,1:4);",
        "((1:3,(4:2,(3:1,2:1):1):1):1,5:4);",
        "((4:3,(3:1,2:1):2):1,(1:2,5:2):2);"
    ]


@pytest.fixture
def seventeen_taxa_nni_neighbours_newick():
    return [
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(7:2,(1:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(2:2,(7:1,1:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(1:3,(6:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,((7:1,2:1):2,(1:2,6:2):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,((((6:3,(1:2,(7:1,2:1):1):1):2,3:5):1,8:6):1,(13:4,14:4):3):3):5,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,(6:3,(1:2,(7:1,2:1):1):1):2):1,3:6):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((13:4,14:4):2,(6:3,(1:2,(7:1,2:1):1):1):3):1,(8:5,3:5):2):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(13:4,14:4):2):1,(6:3,(1:2,(7:1,2:1):1):1):4):3):5,((12:13,(17:8,"
        "16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "(((((((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):2,4:9):1,5:10):5,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):2):1,4:10):5,((12:13,"
        "(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(11:12,(9:11,10:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(17:8,"
        "16:8):5):1,(10:12,(11:11,9:11):1):2):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,(((9:12,(11:11,"
        "10:11):1):1,(17:8,16:8):5):1,12:14):1):1,15:16);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,((12:13,(9:12,"
        "(11:11,10:11):1):1):1,(17:8,16:8):6):1):1,15:16);",
        "(((12:13,(17:8,16:8):5):2,(((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,"
        "14:4):3):3):4,(9:12,(11:11,10:11):1):2):1):1,15:16);",
        "(((9:12,(11:11,10:11):1):3,((12:13,(17:8,16:8):5):1,((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,"
        "2:1):1):1):3):1,(13:4,14:4):3):3):4):1):1,15:16);",
        "((15:15,((12:13,(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):1):1,((5:9,4:9):1,(((8:5,3:5):1,"
        "(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):6);",
        "((((5:9,4:9):1,(((8:5,3:5):1,(6:3,(1:2,(7:1,2:1):1):1):3):1,(13:4,14:4):3):3):5,15:15):1,"
        "((12:13,(17:8,16:8):5):1,(9:12,(11:11,10:11):1):2):2);"
    ]
