# Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>

# Overview
The objective of the program is to develop a distributed database that stores
documents. With a view to optimizing the access to the frequently used documents,
the caching system uses the LRU method (Least Recently Used). This mechanism has
the purpose of evicting the last used key when the cache becomes full. Moreover,
in order for the system to handle more documents more efficiently, the database
uses multiple servers. Ideally, the data should be distributed evenly between
those servers. The current approach uses the Load Balancing with Consistent
Hashing, a frequently used mechanism in distributed systems that has the advantage
of minimal disrruption constraint, meaning minimal transfer of documents between
servers when a new server is added or an existing server is removed. When a new
server is added, only its successor server will distribute documents. When a server
is removed, its documents are distributed to its successor.

# Load Balancer
The Load Balancer in this distributed database system is responsible for efficiently
distributing incoming requests and documents across multiple servers. It leverages
Consistent Hashing, a technique commonly used in distributed systems, to ensure that
the documents are evenly distributed among the servers while minimizing the amount of
data that needs to be moved when the server configuration changes (i.e., when servers
are added or removed).

Features of the Load Balancer:
* Consistent Hashing: This mechanism maps both servers and documents to a hash ring.
Each server is assigned a position on the ring based on its hash value (hash on the
id of the server). Documents are then placed on the server closest to their hash value
in a clockwise direction on the ring. This approach ensures that adding or removing
servers only affects the documents that need to be redistributed to maintain balance,
reducing the impact on the overall system.

* Minimal Disruption: When a new server is added, it only takes over the documents that
would have been assigned to its successor on the ring, minimizing the need for data
movement. Similarly, when a server is removed, its documents are taken over by its
successor. This property is crucial for maintaining system stability and performance
during scaling operations.

* Scalability: The system is designed to scale with minimal impact on the performance
of the existing servers. As the demand grows, additional servers can be seamlessly
integrated without significant data reshuffling, allowing for efficient management of
resources.

* Fault Tolerance: The load balancer's consistent hashing mechanism inherently
provides a degree of fault tolerance. In the event of a server failure, only
the documents assigned to that server need to be redistributed to other servers,
rather than a complete rebalancing of the entire dataset.

* Operational Flow:
Request Handling: When a request to store or retrieve a document is received, the
load balancer hashes the document's key (its name) and determines the appropriate
server on the ring using the consistent hashing algorithm. The hashring is
implemented using an array in ascending order that stores the hashes of the id's
of the servers. Both the insertion of a new server and the request handling based
on hash of the name use binary search for more rapid access. Both the servers
array and the hash ring are implemented as resizable arrays to properly use
memory.

* Document Distribution: New documents are stored on the server that corresponds
to their hash value, ensuring even distribution and efficient use of server resources.
In addition, say the system has multiple servers on the hashring but somehow all of
the documents end up being stored on only one server, due to the hashing mechanism.
In order to avoid this theoretical case, each server has 3 replicas / virtual nodes to
ensure a more even distribution of the documents, increasing the pool of hashes one
server is responsible for. Each server can be represented by multiple points on the
hash ring. All of the three replicas point to the same physical server.

* Server Management: When changes to the server pool occur (i.e., addition or removal
of servers), the load balancer recalculates the positions on the hash ring and adjusts
the distribution of documents accordingly. The adjustments are kept minimal to maintain
performance and reduce the operational overhead.

# Server
The Load Balancer handles multiples servers. Each server has a database (hashtable
implementation), a LRU Cache (implemented using a doubly linked list) and a queue of
requests. Servers receive two types of requests: EDIT and GET. The edit requests are
added in this queue and to be executed in the case of a future GET request. When the
server receives a get request, all of the edit requests stored in this queue are executed.
Afterwards, the GET request itself is executed. Both EDIT and GET functions are static,
meaning they can only be executed from the server.c file to enhance security of the system.

* EDIT:
When an EDIT request is executed, first step is to check if the document is in cache to
be further overriden, if it's not, the database is checked. If the database has it, the
document is added to the cache and its value in the database is overridden. If the cache
is full, the last used key is evicted and added to the database.

* GET:
In a similar fashion, when a GET request is executed, firstly, the cache is checked for
the document, then the database. If the database has it, the document is added to the
cache and if the cache gets full, the last used key is evicted and added to memory.

