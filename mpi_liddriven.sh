cd build
rm ../example_cases/LidDrivenCavity/LidDrivenCavity_Output/*
make && mpirun -np 2 ./fluidchen ../example_cases/LidDrivenCavity/LidDrivenCavity.dat 2 1
# mpirun -np 2 ./fluidchen ../example_cases/ShearFlow/ShearFlow.dat 2 1