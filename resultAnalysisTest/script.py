import pandas as pd
import re
import csv

#read_file = pd.read_csv (r'src\out.log')

with open('src\out.log', 'r') as in_file:
    stripped = (line.strip() for line in in_file)
    #lines = (line.split(" ") for line in stripped if line)
    
    # print(list(stripped))
    r = re.compile(r"^\[INFO\]\tS.*")
    stepEvents = filter(r.match, stripped)
    stepEvents = [l[9:].split(" ") for l in stepEvents]
    
    with open("out.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Node ID", "round", "period", "step", "Simulation time (seconds)", "Chronological time (seconds)"])
        writer.writerows(stepEvents)
