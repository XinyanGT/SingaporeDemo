"""
@Author    Xinyan Yan
@Date      Aug 6, 2014
@Brief     Defining specifics for profiling singapore demo producer process

Subscript starting from 1 in output files
1  row nchunks
2  col nchunks
3  period
4  steps
5  read time
6  build time
7  query time
8  retrieve time
9  write time
10 rough percent
11 exact percent
12 ratio

"""

import math

tryNum = 2

# Construct configurations
row_nchunks = map(lambda x: str(int(math.pow(2,x))), range(6))
col_nchunks = ['1']
col_nchunks.extend(map(lambda x: str(int(3*math.pow(2,x))), range(6)))
chunks = []
for r in row_nchunks:
    for c in col_nchunks:
        chunks.append([r, c])

period = ['1', '10', '50', '100', '200', '400', '700', '1000']

confs = []

for c in chunks:
    for p in period:
        steps = p
        if (int(steps) < 100):
            steps = '100'
        confs.append(c+[p]+[steps])

def prepare(conf):
    pass
    
def getCommandLine(conf):
    command = '~/singapore/app/exe/gen ~/singapore/data/ECEI.norm.bp voltage2 1 1 ' + conf[0] + ' ' + conf[1] + ' ' + conf[2] + ' ' + conf[3] + ' 100 10 -0.06 0.06'
    return command

def getTime(out, err):
    import re
    
    pattern = "Read: (\d+.\d+|\d+)    Index build: (\d+.\d+|\d+)    Query: (\d+.\d+|\d+)    Retrieve: (\d+.\d+|\d+)    Write: (\d+.\d+|\d+)"
    match = re.search(pattern, out)
    perf_time = match.group(1, 2, 3, 4, 5)
    pattern = "Rough: (\d+.\d+|\d+)%    Exact: (\d+.\d+|\d+)%    Ratio: (\d+.\d+|\d+)%"
    match = re.search(pattern, out)
    perf_index = match.group(1, 2, 3)
    # return a tuple
    print str(perf_time + perf_index);
    return  perf_time + perf_index

def getAnci(config):
    return []


