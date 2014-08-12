"""
@Author    Xinyan Yan
@Date      Jun 18, 2014
@Brief     For profiling performance. Dynamically import a module
           which defines profiling specifics, such as command, and 
           how to extract CPU time given command out and err
"""

import subprocess, os, sys

"""
Things need to be defined in each module
  1.Variables:
    tryNum: run times for each configuration
    confs: list of configurations, and each configuration is a list of parameters
  2.Routines
    prepare(conf): prepare run with a configuration
    getComandLine(conf): return command to execute for one configuration
    getTime: extract information from stdout and stderr after the command run
    getAnci: get ancillary statistics from the command run. Must return [] if nothing to return
   
"""
def profile(mod, recordFile):
    pt  = __import__(mod, globals, locals(), ['*'], -1)  # dynamically import the module
    f = open(recordFile, 'w')

    #
    # Iterate through configurations
    #
    for conf in pt.confs:

        # Display information
        print '=============================='
        print 'Configuration: ' + ','.join(conf)

        # Prepare
        pt.prepare(conf)

        # Get command to execute for this configuration
        commandLine = pt.getCommandLine(conf)
        
        #
        # Iterate through trials
        #
        for i in range(pt.tryNum):

            # Execute the command
            process = subprocess.Popen(commandLine, shell=True, 
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            out, err = process.communicate()
            tStat = pt.getTime(out, err)   # time statistics, which may also contains some anci value(s)
            aStat = pt.getAnci(conf);          # ancillary statistics

            # Display on the screen
            displayStr = "Iteration: " + str(i) + '\t Time and Anci: ' + str(tStat)
            displayStr += "\t Ancillary statistis: " + str(aStat)
            print displayStr

            # Write to record file
            f.write(','.join(conf) + ',' + ','.join(tStat) + ',' + ','.join(aStat) + '\n')

            # Flush now
            f.flush()

    # Clean
    f.close()

if __name__ == "__main__":
    mod = sys.argv[1]
    recordFile = sys.argv[2]
    profile(mod, recordFile)


