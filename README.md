**Name: Tudor Cristian-Andrei**

**Class: 311CAa**

## <font color="#917FB3"> Load Balancer - 2nd Homework </font>
<br>

### The flow of the program
<br>

My <font color="#9384D1">Load Balancer</font> is made up of 2 main parts: **an array of servers**, and **the hashring**. The most important part of the Load Balancer is the **hashring**, thought. It is an array of structures, **ring_t**, but I like to call them just **rings**. In my view, the **hashring** is an infinite chain, made from those **rings**, connected at the ends, so there is no end. The <font color="#ECC9EE">**ring_t**</font> looks like this: <br> 
```c 
struct ring_t {
    unsigned int hash;
    unsigned int server_id;
}
```
> As you see, the rings are made just to store together both the **hash** and the **server_id**. In this way, you have acces to the ID of the server at each point in time, even if there is a replica of the original server represented on the **hashring**.

The <font color="#9384D1">Load Balancer</font> looks like this (I changed a little bit the name of the structures, I hope you don't mind):
```c
struct load_balancer_t {
    server_t *servers;
    unsigned int max_servers;
    unsigned int num_servers;
    ring_t *hashring;
    unsigned int hashring_size;
}
```
> Here, you can see a field named **max_servers** which allows me to resize the arrays. The memory isn't increasing from server to server, but I increase the memory from 10 to 10. I have to be honest and say that this was easier, but I don't think it is a bad idea (I will get back to this in the last section of the README). The **hashring_size** field may not seem necessary at all, but it helped to the implementations, especially to adding and removing operations, performed on servers.

I didn't say a word about the proper servers until now. The <font color="#ECC9EE">**server_memory_t**</font> is just a Hashtable renamed. For the servers used in <font color="#9384D1">Load Balancer</font>, I defined a new struct, <font color="#ECC9EE">**server_t**</font> which describes a server with 2 components: a memory / storage, and an ID. I couldn't add this in the <font color="#ECC9EE">server_memory_t</font> struct because of the *init_server_memory* function, which has no parameter, so I couldn't assign an ID at initialization (again, I will get back to this in the last section of the README).

> I have to say that my implementation is based more on the **hashring**, than the array of servers. Almost every operations is performed on the **hashring**, and if it is performed on the actual array, it is first performed on the **hashring**. Details about those operations are provided in the next section.

### Structure of the code

The code is divided into 4 main parts:
1. <font color="#9384D1">Data Structuring Part</font> <br> <font color="#ECC9EE">data_structs.h <br> datastruct_funcs.c / datastruct_funcs.h</font> <br> The first one is the header file that contains all the structures used in the code, some of them are detailed above. The second one, contains the functions related to Linked Lists and Hashtable.

> The comments are missing from those files because the Linked List implementation doesn't belongs to me, it is taken from a lab, and the Hashtable is my code from the lab, but we get used to those and I though I don't need to explain the functions. I have to mention that the **pair_t** struct is the same as **info_t**, but I like it more this way.

2. <font color="#9384D1">Server Part</font> <br> <font color="#ECC9EE">server.h / server.c </font> <br> Because my server is just a hashtable, this was the easiest part of the project, because I just call Hashtable functions in each server function. For example, *server_store* is just the **put** operation, *server_retrive* is the **get** operation, and *server_remove* is the **remove entry** from hashtables.

3. <font color="#9384D1">Load Balancer Part</font> <br> <font color="#ECC9EE">load_balancer.h / load_balancer.c </font> <br> Here was the actual effort.
> ### Initialization of a <font color="#9384D1">**Load Balancer**</font> 
> It is a simple task, I explained the resize property early, but it's the only worth mentioning aspect. At the begging, the <font color="#9384D1">**Load Balancer**</font> only supports 10 servers, and 30 elements on **hashring**. The hashring is always triple size.

> ### Adding a server
> Whenever a server is added, it is connected at the end of the already existing servers. The order doesn't matter in the **array of servers**. There are changes on the **hashring**. The 3 new hashes are ordered, and connected at the end of the hashring. After this, there is a concatenation between the 3 new elements, and the old **hashring**. The **hashring** is always sorted. <br> After the hashes are inserted in the **hashring**, the system is rebalanced, not the entire system, just the servers next to the new server (I'm refering to **hashring** positions)
>> It is worth mentioning that I use and auxiliary array for concatenation.

> ### Removing a server
> First, all the hashes related to the server are deleted from the **hashring**. This will determine the function that finds the server where to store a key, to ignore his existence in the <font color="#9384D1">Load Balancer</font>. Then, the server is freed as the objects are transfering to a new place. <br> As I said early, the order of the actual servers doesn't matter. So, when I delete a server, I perform a swap between the last one in the array, and the actual one, then deleting the last one. (I will bring this in discussion in the last part.)

> ### Store and retrieve
> For both operations, the hash of the key is generated, and a function finds the best place to put the key-value pair (in the case of retrive it acts the same, but I use it different). After knowing the server, it is just a simple store / retrieve operation on server.

> ### Free <font color="#9384D1">**Load Balancer**</font>
> The removing server functions ends with a call to *delete_server* functions, that works even on servers that are full of keys. This task consist in looping this function. I delete all the servers within the array, then free the structure properly.

4.  <font color="#9384D1">Main Part</font> <br> <font color="#ECC9EE">main.c </font>

> There is an additional file, <font color="#ECC9EE">utils.c </font>, where I put my macros.

## Upgrades
Here, I will say a couple of words about what could have been done better, or about something that I completely forgot about.
> ### Resizing the array
> I forgot about shrinking the array when removing too many servers, without adding others. My initial plan for this was to have the maximum difference between max_servers and num_servers of 10. It is not hard to make this change, but I completely forgot.

> ### Binary Search
> I could have used an more efficient way to find hashes in the **hashring**, like binary search.

> ### The remapping objects system
> I think there is a more efficient way to remap the objects, I had a couple of tries, but this was the best version, which passed the tests.

> ### Sorting in place
> Maybe, for big Load Balancers, my sorting will crash the entire system, because it will try to allocate to much memory, for one more **hashring**.

<br><br><br><br>

> Note: I'm sorry for my bad English, this is the first time I did this. Have a nice day!