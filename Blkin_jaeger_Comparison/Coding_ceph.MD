# Coding in Ceph with Container
## Setup environemnt (For MAC)
 - git clone --recurse-submodules -j8 <git@github.com:Bowenislandsong/ceph.git> (-j8 optional) (this make sure you grab everything including thoes directories with a link)                            
 - docker run -ti -v <where it is at>:/Ceph centos
 - cd Ceph 
 - ./install-deps.sh
 - ./do_cmake.sh
 - cd build
 - make
 - make install 

God compiling takes forever, NVM