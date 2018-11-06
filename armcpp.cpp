本文目的：
致力于petalinux支持arm-linux-g++,语言cpp，支持c++11

步骤：
建立cpp工程，(本质使用的是arm-linux-g++编译器)
修改Makefile
修改bb文件

常用命令
----
petalinux-build -c adtest  -x build
petalinux-build -c adtest  -x install
find build/ -name adtest
cp build/tmp/work/cortexa9hf-neon-xilinx-linux-gnueabi/adtest/1.0-r0/package/usr/bin/adtest /home/pp/winshare/



 petalinux-create -t apps --template c --name myapp --enable
 petalinux-build -c myapp -x build
 petalinux-build -c myapp -x install
 find build/ -name myapp
 cp build/tmp/work/cortexa9hf-neon-xilinx-linux-gnueabi/myapp/1.0-r0/package/usr/bin/myapp /home/pp/winshare/
 
 
Makefile
----
APP = myapp

# Add any other object files to this list below
APP_OBJS = myapp.o

CC      := arm-linux-gnueabihf-g++
CXX      := arm-linux-gnueabihf-g++
LDLIBS    += -lpthread 
LDFLAGS += -Wl,--no-as-needed -lpthread -L/home/pp/company/code1030/test/iiodproj/build/tmp/sysroots/plnx_arm/usr/lib -liio -lxml2 -lz
DEFINES   := 
INCLUDE   += -I./
CFLAGS    +=   -I/home/pp/company/code1030/test/iiodproj/build/tmp/sysroots/plnx_arm/usr/include/ -g -Wall -O2 --std=c++11 -pthread -liio $(DEFINES) $(INCLUDE)
CXXFLAGS += $(CFLAGS)


all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

clean:
	-rm -f  $(APP) *.elf *.gdb *.o
  
  
test.cpp
----
#include <iostream>
#include <utility>
#include <vector>
#include <string>
 
int main()
{
    std::string str = "Hello";
 
   std::vector<std::string> v;
 
    // 使用 push_back(const T&) 重载，
    // 表示我们将带来复制 str 的成本
    v.push_back(str);
    std::cout << "After copy, str is \"" << str << "\"\n";
 
    // 使用右值引用 push_back(T&&) 重载，
    // 表示不复制字符串；而是
    // str 的内容被移动进 vector
    // 这个开销比较低，但也意味着 str 现在可能为空。
    v.push_back(std::move(str));
    std::cout << "After move, str is \"" << str << "\"\n";
 
    std::cout << "The contents of the vector are \"" << v[0]
                                         << "\", \"" << v[1] << "\"\n";
     std::cout << "Hello, world!" << "cpp" << std::endl;
}

