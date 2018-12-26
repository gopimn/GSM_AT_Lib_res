/**
 * \page            page_appnote Application note
 * \tableofcontents
 *
 * \section         sect_clone_get_started Clone repository and getting started
 *
 * Library development is fully hosted on Github and there is no future plans to move to any other platform.
 *
 * There are `2` repositories
 *
 *	- <a href="https://github.com/MaJerle/GSM_AT_Lib"><b>GSM_AT_Lib</b></a>: Source code of library itself.
 *	    - Repository is required when developing final project
 *
 *	- <a href="https://github.com/MaJerle/GSM_AT_Lib_res"><b>GSM_AT_Lib_res</b></a>: Resources, development code,
 *		documentation sources, examples, code snippets, etc.
 *      - This repository uses `GSM_AT_Lib` repository as `submodule`
 *	    - Repository is used to evaluate library using prepared examples
 *
 * \subsection      subsect_clone_res Clone resources repository with examples
 *
 * Easiest way to test the library is to clone resources repository.
 *
 *  - Download and install `git` if not already
 *  - Open console and navigate to path in the system to clone repository to. Use command `cd your_path`
 *  - Run `git clone https://github.com/MaJerle/GSM_AT_Lib_res` command to clone repository
 *  - Enter into newly cloned folder using `cd GSM_AT_Lib_res`. Now we are inside working git directory
 *  - Run command `git submodule update --init --recursive` to download and update all submodules
 *  - Navigate to `examples` directory and run favourite example
 *
 * \subsection      subsect_clone_lib Clone library only
 *
 * If you are already familiar with library and you wish to include it in existing project, easiest way is to clone library repository only.
 *
 *  - Download and install `git` if not already
 *  - Open console and navigate to path in the system to clone repository to. Use command `cd your_path`
 *  - Run `git clone https://github.com/MaJerle/GSM_AT_Lib` command to clone repository
 *
 * \section         sect_project_examples Example projects
 *
 * \note            Examples are part of `GSM_AT_Lib_res` repository. Refer to \ref subsect_clone_res
 *
 * Several examples are available to show application use cases. These are split and can be tested on different systems.
 *
 * \subsection      subsect_project_examples_win32 WIN32 examples
 *
 * Library is developed under WIN32 system. That is, all examples are first developed and tested under WIN32, later ported to embedded application.
 * Examples come with <b>Visual Studio</b> project. You may open project and directly run the example from there.
 *
 * \note            It may happen that Visual Studio sets different configuration on first project load and this may lead to wrong build and possible errors.
 *                  Active configuration must be `Debug` and `Win32 or x86`. Default active build can be set in project settings.
 *
 * \par             SIM800 development board
 *
 * For development purposes, `SIM800` board is used with external battery + virtual COM port device (such as FTDI)
 *
 * \par             System functions for WIN32
 *
 * Required system functions are based on "windows.h" file, available on windows operating system. Natively, there is support for:
 * 
 *  - Timing functions
 *  - Semaphores
 *  - Mutexes
 *  - Threads
 *
 * The last part are message queues which are not implemented in Windows OS.
 * Message queues were developed with help of semaphores and dynamic memory allocatations.
 * System port for WIN32 is available in `src/system/gsm_sys_win32.c` file.
 *
 * \par             Low-level communication between SIM800 board and WIN32
 *
 * Communication with SIM800 board hardware is using virtual files for COM ports.
 * Implementation of low-level part (together with memory allocation for library) is available in `src/system/gsm_ll_win32.c` file.
 *
 * \note            In order to start using this port, user must set the appropriate COM port name when opening a virtual file. 
 *                  Please check implementation file for details.
 *
 * \subsection      subsect_project_examples_arm_embedded ARM Cortex-M examples
 *
 * Library is indendant from CPU architecture, meaning we can also run it on embedded systems. 
 * Different ports for `FreeRTOS` operating system and `STM32` based microcontrollers are available too.
 *
 *  <table>
 *      <caption>STM32 boards and pinouts for tests</caption>
 *      <tr><th> 						<th colspan="4">GSM target settings	<th colspan="5">Debug settings
 *      <tr><th>Board name				<th>UART 	<th>MTX <th>MRX <th>RST <th>UART    <th>MDTX<th>MDRX<th>DBD     <th>Comment
 *      <tr><td>\b STM32F429ZI-Nucleo   <td>USART2 	<td>PD5 <td>PD6 <td>PD1 <td>USART3  <td>PD8 <td>PD9 <td>921600  <td>OBSTL
 *  </table>
 *
 * 	- \b MTX: MCU TX pin, other device RX pin
 * 	- \b MRX: MCU RX pin, other device TX pin
 * 	- \b RST: Reset pin from GSM device, connected to MCU
 * 	- \b MDTX: MCU Debug TX pin, other device RX pin
 * 	- \b MDRX: MCU Debug RX pin, other device TX pin
 * 	- \b DBD: Debug UART baudrate
 * 	- \b OBSTL: On-Board ST-Link USB virtual COM port
 *
 * \note            All examples for STM32 come with ST's official free development studio.
 *
 * \section         sect_porting_guide Porting guide
 *
 * \subsection      subsect_sys_arch System structure
 *
 * \image html system_structure.svg System structure organization
 *
 * We can describe library structure in `4` different layers:
 *
 *  - <b>User application</b>: User application is highest layer where entire code is implemented by user
 *      and where GSM AT library API functions are called from
 *
 *  - <b>GSM AT middleware</b>: GSM AT middleware layer consists of API functions,
 *      thread management functions and all utilities necessary for smooth operation.
 *
 *  - <b>System functions</b>: Layer where system dependant functions must be implemented,
 *      such as current time in milliseconds and all OS dependant functions for:
 *      - Managing threads
 *      - Managing semaphores
 *      - Managing mutexes
 *      - Managing message queues
 *
 *      More about this part can be found in \ref GSM_SYS section.
 *
 *  - <b>AT port communication functions</b> or <b>GSM LL</b>: Part of code where user must take care
 *      of sending and receiving data from/to GSM AT lib to properly handle communication between
 *      host device and GSM device.
 *      - User must assign memory for memory manager in this section.
 *          Check \ref GSM_MEM and \ref GSM_LL sections for more information.
 *
 *      More about this part can be found in \ref GSM_LL section.
 *      
 *      Together with this section, user must implement part to input the received data from AT port.
 *
 *  - <b>GSM physical device</b>: Actual ESP8266 or ESP32 device
 *
 * \subsection      subsect_port_implementation Implementation specific part
 *
 * Before usage, user must implement all functions in \ref GSM_LL section
 * as well as take care of proper communication with GSM device in \ref GSM_LL section.
 *
 * \note            For more information about how to port, check sections accordingly
 *
 * \section         sect_config Library configuration
 *
 * To make library as efficient as possible, different configuration parameters are available
 * to make sure all the requirements are met for different purposes as possible.
 *
 * A list of all configurations can be found in \ref GSM_CONF section.
 *
 * \subsection      subsect_conf_file Project configuration file
 *
 * Library comes with `2` configuration files:
 *
 *  - Default configuration file `gsm_config_default.h`
 *  - Project template configuration file `gsm_config_template.h`
 *
 * When project is started, user has to rename template file to `gsm_config.h`
 * and if required, it should override default settings in this file.
 *
 * Default template file comes with something like this:
 *
 * \include         _example_config_template.h
 *
 * In case user wants to increase default buffer size_t for received data,
 * a file should be modified to something similar like code below:
 *
 * \include         _example_config.h
 *
 * \note            Important notes:
 *                      - Always do default settings modifications in user custom `gsm_config.h` file
 *                          which was previously renamed from `gsm_config_template.h`,
 *                      - Always include `gsm/gsm_debug.h` before any custom settings and
 *                          `gsm/gsm_config_default.h` after custom settings
 *
 * \section         sect_thread_comm Inter-thread communication
 *
 * In order to have very effective library from resources point of view,
 * an inter-thread communication was introduced.
 *
 * \image html thread_communication.svg Inter-Thread communication between user and library.
 *
 * Library consists of 2 threads working in parallel and bunch of different user threads.
 *
 * \subsection      subsect_thread_user User thread(s)
 *
 * User thread is a place where user communicates with GSM AT library.
 * When a new command wants to be executed to GSM device, user calls appropriate API function which will do following steps:
 *
 *  - Allocate memory for command message from memory manager
 *  - Assign command type to message
 *  - Set other parameters, related or required to command
 *  - If user wants to wait for response (blocking mode), then create system semaphore `sem` and lock it immediatelly
 *  - Send everything to producing message queue which is later read in producing thread
 *  - If user don't want blocking mode, return from function with status OK
 *      otherwise wait for semaphore `sem` to be released from producing thread
 *      - When `sem` semaphore is locked, user thread may sleep and release resources for other threads this time
 *  - If user selects blocking mode, wait for response, free command memory in memory manager and return command response status
 *
 * User may use different threads to communicate with GSM AT lib at the same time since memory manager
 * is natively protected by mutex and producing queue is protected from multiple accesses by OS natively.
 *
 * \subsection      subsec_thread_producer Producer thread
 *
 * Producer threads reads message queue with user commands and sends initial AT command to AT port.
 * When there is no commands from user, thread can sleep waiting for new command from user.
 *
 * Once there is a command read from message queue, these steps are performed:
 *
 *  - Check if processing function is set and if command is valid
 *  - Locks `sync_sem` semaphore for synchronization between processing and producing threads
 *  - Sends initial AT command to AT port according to command type
 *  - Waits for `sync_sem` to be ready again (released in processing thread)
 *  - If command was blocking, set result and unlock command `sem` semaphore
 *  - If command was not blocking, free command memory from memory manager
 *
 * \subsection      subsec_thread_process Process thread
 *
 * Processing thread reads received data from AT port and processes them.
 *
 * If command is active and received data belongs to command, they are processed according to command.
 * If received data are not related to command (such as received network data `+IPD`),
 * they are also processed and callback function is immediatelly called to notify user about received data.
 *
 * Here is a list of some URC (Unsolicited Result Code) messages:
 *  
 *  - Received network data `+IPD`
 *  - Connection just active `+LINK_CONN`
 *  - Station disconnected from access point `WIFI DISCONNECT`
 *  - Station connected to access point `WIFI CONNECTED`
 *  - ...
 *
 * All these commands must be reported to user. To do this, callback is triggered to notify user.
 *
 * \section         sect_block_nonblock Blocking and non-blocking commands
 *
 * Every command (except if noted otherwise) can be executed in blocking or non-blocking mode.
 *
 * \subsection      subsect_blocking Blocking mode
 *
 * When blocking mode is selected, function will block thread execution until response is received
 * and ready for further processing. When the function returns, user has known result from GSM device.
 * Linear programming style may be applied.
 *
 * \include         _example_blocking_pseudo.c
 *
 * \warning         When user wants to send command from callback function,
 *                  it is mandatory to call it in non-blocking way, otherwise you may enter dead-lock
 *                  and your program will stop in this position forever.
 *
 * \subsection      subsect_nonblocking Non-blocking mode
 *
 * In non-blocking mode, command is created, sent to producing message queue and function returns without waiting for response from device.
 * This mode does not allow linear programming style, because after non-blocking command, callback function is called.
 *
 * \note            As of now, fully implemented callbacks are implemented for \ref GSM_CONN section onlyl
 *                  because these are used the most and therefore most focus was applied to this section.
 *
 * \note            When user wants to send command from callback function,
 *                  this is the only allowed way to do it. Every command must be called in
 *                  non-blocking way from callback function.
 *
 * Pseudo code example for non-blocking API call. Full example for connections API can be found in \ref GSM_CONN section.
 *
 * \include         _example_nonblocking_pseudo.c
 *
 */