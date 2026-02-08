# Traffic Simulation Demo

This project marks my first foray into modern C++ development. It is a very simple traffic simulator. While my initial idea was an entire grid of streets and avenues with
connecting intersections, and cars having a myriad of origins and destinations, that idea proved too large for the requirements of this demo. So I pared it way back,
to only a single intersection at the center of four connecting streets. The intersection runs traffic lights for all four directions. Cars originate at the end of each street,
drive straight across the intersection (when the light allows!) and arrive at their destination at the opposite end of the map. I created a crude visualizer that shows the cars
coming and going, as well as a graph showing the total traffic volume. This will only work on Windows.

I used this project to learn about memory management and smart pointers, object ownership and lifespan, concurrency, and const-correctness. I expect that there 
are improvements that can be made to the code, but it was a useful exercise and I am much more comfortable with C++ today than I was a week ago.

I also set up a Testing configuration which uses Google Test as a framework. The libraries (v1.17.0) were downloaded from https://github.com/google/googletest and built myself.
