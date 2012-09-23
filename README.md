## Building in Eclipse ##

* First you need an ARM toolchain. I used **Yagarto**: http://www.yagarto.de. Install it as per the instructions.
* Now download Eclipse and unpack it somewhere. At the time of writing Eclipse 3.7 was the latest stable version.
* To work with ARM projects in Eclipse you need a few plugins:
   + **Eclipse C Development Tools** (CDT) (available via *Help > Install new Software*)
   + **Zylin Embedded CDT Plugin** (http://opensource.zylin.com/embeddedcdt.html)
   + **GNU ARM Eclipse** (http://sourceforge.net/projects/gnuarmeclipse/)
   + If you want to hook up an SWD debugger you also need the **GDB Hardware Debugging** plugin (Also available via *Install new Software*) and OpenOCD installed (available via MacPorts on Mac OS X) on your machine.
* Now clone the project to your harddrive:

		git clone https://github.com/fiendie/baseflight.git

* Create a new C project in Eclipse and choose ARM Cross Target Application and your ARM toolchain (Yagarto in my case)
* Import the git project into the C project in Eclipse via *File > Import > General > File System*
* Activate Git via *Project > Team > Share Project*
* Switch to master branch in Eclipse (*Project > Team > Switch To > master*)
* The next thing you need to do is adjust the project configuration. There is a makefile included that works but you might want to use GNU ARM Eclipse's automatic makefile generation. Open the Project configuration and go to *C/C++ Build > Settings*
   + Under *Target Processor* choose "cortex-m3"
   + Under *ARM Yagarto Mac OS Linker > General* (or whatever toolchain you chose)
      - Browse to the Script file *stm32_flash.ld*
      - Uncheck "Do not use standard start files"
      - Check "Remove unused sections"
   + Under *ARM Yagarto Mac OS Linker > Libraries*
      - Add "m" for the math library
   + Under *ARM Yagarto Mac OS Compiler > Preprocessor* add the following 2 items to "Defined Symbols":
      - STM32F10X_MD
      - USE_STDPERIPH_DRIVER
   + Under *ARM Yagarto Mac OS Compiler > Directories* add the following 3 items
      - ${workspace_loc:/${ProjName}/lib/CMSIS/CM3/CoreSupport}
      - ${workspace_loc:/${ProjName}/lib/CMSIS/CM3/DeviceSupport/ST/STM32F10x}
      - ${workspace_loc:/${ProjName}/lib/STM32F10x_StdPeriph_Driver/inc}
   + Under *ARM Yagarto Mac OS Compiler > Miscallaneous* add the following item to "Other flags":
      - -fomit-frame-pointer	
   + The code in the support directory is meant for you host machine and must not be included in the build process. Just right-click on it to open its properties and choose "Exclude from build" under *C/C++ Build > Settings*
* Try to build the project via *Project > Build Project*
* _Note_: If you're getting "...could not be resolved" errors for data types like int32_t etc. try to disable and re-enable the Indexer under *Project > Properties > C/C++ General > Indexer*