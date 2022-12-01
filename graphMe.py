import pandas as pd 
import numpy as np
import matplotlib.pyplot as plt

def read_csv(filename: str):
    return pd.read_csv(filename)

df = read_csv('2cars7seats.csv')
dg = read_csv('4cars7seats.csv')
dh = read_csv('6cars7seats.csv')

np_time = np.array(df["time"])
np_arrived = np.array(df["arrived"])

np_c2_wait = np.array(df["waitline"])
np_c4_wait = np.array(dg["waitline"])
np_c6_wait = np.array(dh["waitline"])

np_c2_rejected = np.array(df["rejected"])
np_c4_rejected = np.array(dg["rejected"])
np_c6_rejected = np.array(dh["rejected"])

fig, ax = plt.subplots()
plt.plot(np_time, np_arrived, "r-D", markersize=4, 
        linewidth='.5',label="25,45,35,25")
plt.title("People Arrival")
plt.ylabel("Arrival #")
plt.ylim([0,85])
plt.xlabel("Time (minutes)")
plt.xlim([0,600])
plt.grid(linestyle='dashed')
plt.legend(loc='upper right')
plt.savefig('People_Arrived.png')

fig, ax = plt.subplots()
plt.plot(np_time, np_c2_wait, 'r-D', markersize=4, 
        linewidth='.5', label="car = 2, seats = 7")
plt.plot(np_time, np_c4_wait, 'g-+', markersize=4, 
        linewidth='.5', label="car = 4, seats = 7")
plt.plot(np_time, np_c6_wait, 'b-s', markersize=4, 
        linewidth='.5', label="car = 6, seats = 7")
plt.title("Waiting Queue")
plt.ylabel("# if persons in waiting")
plt.xlabel("Time (minutes)")
plt.ylim([0,1000])
plt.xlim([0,600])
plt.grid(linestyle='dashed')
plt.legend(loc='upper right')
plt.savefig('Waiting_Queue.png')

fig, ax = plt.subplots()
plt.plot(np_time, np_c2_rejected, 'r-D', markersize=4, 
        linewidth='.5', label="car = 2, seats = 7")
plt.plot(np_time, np_c4_rejected, 'g-+', markersize=4, 
        linewidth='.5', label="car = 4, seats = 7")
plt.plot(np_time, np_c6_rejected, 'b-s', markersize=4, 
        linewidth='.5', label="car = 6, seats = 7")
plt.title("Go Away People")
plt.ylabel("# if persons in waiting")
plt.xlabel("Time (minutes)")
plt.ylim([0,70])
plt.xlim([0,600])
plt.grid(linestyle='dashed')
plt.legend(loc='upper right')
plt.savefig('Rejected.png')

