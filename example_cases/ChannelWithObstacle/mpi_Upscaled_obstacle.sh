clear # clear terminal
rm ../example_cases/ChannelWithObstacle/ChannelWithObstacleUpscaled_Output/* #remove old output files
make && mpirun -np 1 ./fluidchen ../example_cases/ChannelWithObstacle/ChannelWithObstacleUpscaled.dat 1 1