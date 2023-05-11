set(SRC_LIST 
    rpctestapplication.cc 
    rpctestconfig.cc 
    rpctestprovider.cc
    rpcheader.pb.cc 
    rpctestchannel.cc
    logger.cc
    zookeeperutil.cc)
add_library(rpctest ${SRC_LIST})
target_link_libraries(rpctest muduo_net muduo_base pthread zookeeper_mt)