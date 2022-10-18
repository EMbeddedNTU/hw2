import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import threading

data_lst = [60, 59, 49, 51, 49, 52, 53]

fig, ax = plt.subplots()
xs = []
ys = []
start_ms = time.time()*1000

def get_data():
    return data_lst[-1]

def animate(i, xs:list, ys:list):
    # grab the data from thingspeak.com

    # Add x and y to lists
    xs.append(time.time()*1000 - start_ms)
    ys.append(data_lst[-1])
    # Limit x and y lists to 10 items
    xs = xs[-10:]
    ys = ys[-10:]
    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys)
    # Format plot
    ax.set_ylim([0,150])
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.20)
    ax.set_title("title")
    ax.set_xlabel('time(ms)')
    ax.set_ylabel('data')

def job(args):
    print(args)
    global data_lst
    while True:
        # print(data_lst[-1])
        data_lst += [data_lst[0]] 
        data_lst = data_lst[-10:] 
        time.sleep(1)


t = threading.Thread(target = job, args = [10])
t.start()

# Set up plot to call animate() function every 1000 milliseconds
ani = animation.FuncAnimation(fig, animate, fargs=(xs,ys), interval=100)
plt.show()    


