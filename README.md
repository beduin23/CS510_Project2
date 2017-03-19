# CS510_Project2
To build llvm, we have to follow this steps:

## Step1: 
First, we need to create the build directory in the directory project2, where there are two other directories name llvm and testcase
```
mkdir build && cd build
```
## Step2:
Then from the build directory, we have to run cmake, there is a script in the project2 directory , we can simply run this script, but before that, we need to check whether we have execution permission to that script
```
chmod +x cmake_script.sh
cd build
../cmake_script.sh
 ```

or we can just give the command of cmake instead of running the script:
```
cmake -GNinja \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DLLVM_ENABLE_ASSERTIONS=ON \
-DLLVM_BUILD_TESTS=OFF \
-DLLVM_BUILD_EXAMPLES=OFF \
-DLLVM_INCLUDE_TESTS=OFF \
-DLLVM_INCLUDE_EXAMPLES=OFF \
-DBUILD_SHARED_LIBS=on \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DCMAKE_C_FLAGS="-fstandalone-debug -fuse-ld=gold" \
-DCMAKE_CXX_FLAGS="-fstandalone-debug -fuse-ld=gold" \
 ../llvm
```

## Step 3: 
For building, we are needed to have ninja, if ninja is not installed, we have to remove the -GNinja option from the cmake script
```
ninja
```
## Step 4: 
Next, we have to compile test case with my pass, the general command for this is
```
/path/to/clang   /path/to/testcase/tc1.c    -fsanitize=cgraph
```

For example, to check testcase tc1.c within the testcase directory, we have to give the following command:
```
../build/bin/clang tc1.c -fsanitize=cgraph
``` 

## Step 5: 
The previous command will output in the console, it will also generate a dot file of the program, to check the dot file, we can convert it from dot to png using the following command. For this command we need to install graphviz.
```
dot -Tpng cgraph.dot > cgraph.png
```
