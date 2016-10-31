#include <iostream>
#include "cl.hpp"
#include <chrono>

int main (void)
{
    //Platform
    std::vector<cl::Platform> all_Platforms;
    cl::Platform::get(&all_Platforms);

    if(all_Platforms.size()==0)
    {
        std::cout<<" No platfotms found. Check OpenCL instalation!\n";
        exit(EXIT_FAILURE);
    }

    cl::Platform default_platform = all_Platforms[0];
    std::cout<<"Using platfom: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<std::endl;

    //Devices
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, & all_devices);
    if(all_devices.size()==0)
    {
        std::cout<<"No devices found Check OpenCL is properly installed on your system"<<std::endl;
        exit(EXIT_FAILURE);
    }

    cl::Device default_device = all_devices[0];
    std::cout<<"Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<std::endl;

    //Context
    cl::Context context({default_device});

    //Sources
    cl::Program::Sources sources;

    // kernel calculates for each element C=A+B
    std::string kernel_code=
            "   void kernel simple_addFunction(global const int* A, global const int* B, global int* C)"
            "   {"
            "       C[get_global_id(0)] = A[get_global_id(0)]+B[get_global_id(0)];"
            "   }";

    //Build our kernels
    sources.push_back({kernel_code.c_str(),kernel_code.length()});

    cl::Program program(context,sources);
    if(program.build({default_device})!=CL_SUCCESS)
    {
       std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
        exit(EXIT_FAILURE);
    }


    //Create buffers on device for A,B,C arrays
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*1000000);
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*1000000);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*1000000);

    int *A = new int[1000000];//{1,1,1,1,1,1,1,1,1,1};

    int *B = new int[1000000];//{0,1,2,3,4,5,6,7,8,9};

    //fill the 2 arrays with data
    for (int i=0;i<1000000;i++)
    {
        A[i]=i;
        B[i]=i+1;
    }

    //create queue to which we will push commands for the device.
    cl::CommandQueue queue(context,default_device);

    //write/copy arrays A and B to the device
    queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*1000000,A);
    queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*1000000,B);

    //run the kernel
    cl::KernelFunctor simple_add(cl::Kernel(program,"simple_addFunction"),queue,cl::NullRange,cl::NDRange(1000),cl::NullRange);
    simple_add(buffer_A,buffer_B,buffer_C);

    int *C = new int[1000000];
    //    int C[10];


    //traditional - non-parallel way of computing the sum
    auto start = std::chrono::system_clock::now();
    for (int i=0;i<1000000;i++)
    {
        C[i]=A[i]+B[i+1];
    }

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<"No OpenCL: time elapsed:"<< elapsed.count() << " microseconds" << '\n';


    //print memory C on our Device and transfer back to Host

    start = std::chrono::system_clock::now();
    /* do some work */
        queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*1000000,C);

    end = std::chrono::system_clock::now();
    // this constructs a duration object using milliseconds
    elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<"OpenCL: time elapsed:"<< elapsed.count() << " microseconds" << '\n';


    std::cout<<"  result: \n";
    for(int i=0;i<1000000;i++)
    {
        //std::cout<<"A["<<i<<"] +"<<" B["<<i+1<<"] = "<<C[i]<<" \n";
    }

    delete A;
    delete B;
    delete C;

    return 0;
}
