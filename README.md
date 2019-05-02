# Magnus' Ascend stuff

- Tf: static tf transform
    There is a static tf transform launch file, atm containing static transformation from camera to drone, dronemap to drone and dronemap to map.
    Thease are indeed static and the parameters needs to be defined, Typicaly (Transform[x,y,z]) and (Rotation[r,p,y]).
    Thease are defined as x y z r p y in the launch file named drone_tf.launch in the package drone tf.


- The package drone_tf har a node called dronestatic.cpp
  - There is also a launch file, called "drone_tf.launch"
  
#
- The package/code for publishing zed pointclouds are heavely based on https://github.com/stereolabs/zed-pcl, so if there are errors of questings to what is happening that is the place to go.
