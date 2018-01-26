# Project 2: Implementing “always on” end-to-end tracing in the Ceph distributed storage service

Mentors:  Mania Abdi (mania.abdi287@gmail.com), Raja Sambasivan (rrs@bu.edu), Peter Portante (pportant@redhat.com)

Golsana Ghaemi

Bowen Song (sbowen@bu.edu)

Oindrilla Chatterjee

Aditya Singh

Useful skills:
C, C++, Python
Familiarity with distributed storage services (e.g., AWS, S3).  
Min-max team size: 2-4
Expected project hours per week (per team member): 6-8
Will the project be open source?: Yes

Ceph is an extremely popular open-source distributed storage service used within many cloud environments.  For example, we use it extensively within the Massachusetts Open Cloud to store data for our users.  It is an immensely complex service consisting of over a million lines of code.  A single ceph deployment may consist of numerous storage nodes and various libraries and gateways to support different types of storage (block, object, filesystem).  While Ceph has blkin tracing today, it is not continuously enabled and is fairly rudimentary (it does not capture concurrency, synchronization, or allow critical paths to be extracted).   As a step toward understanding important performance and correctness problems in this important service, there is a need to implement an “always-on,” sophisticated end-to-end tracing system within it.


Project tasks: In this project, we will expand on existing efforts to implement end-to-end tracing within Ceph(http://docs.ceph.com/docs/master/dev/blkin/).  The goal will be to create an “always-on” tracing system with low overhead.  We will drive Ceph with various workloads and use the resulting traces to understand Ceph’s performance characteristics under various scenarios.  We will also use the traces to understand performance issues a team at BU working on Ceph has encountered.
Skills students will learn

Ceph
How to implement and use end-to-end tracing, which is becoming the de facto method to analyze and debug distributed applications at large technology companies.  
A thorough understanding of existing open-source end-to-end tracing infrastructures, Uber’s Jaeger and Mirantis’s OSProfiler.  
How to apply machine-learning techniques to derive insights from rich trace data.  
Depending on the project, familiarity with commonly-used distributed applications and microservices.  Examples include Ceph and OpenStack.
