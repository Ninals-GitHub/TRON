/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by T-Engine Forum
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Last modified at 2012/11/30 by T-Engine Forum.
 *
 *----------------------------------------------------------------------
 */

==============================================================================
T-Kernel 2.0 for T-Engine Reference Board and Eclipse Development Environment
==============================================================================

------------------------------------------------------------------------------
 Contents
------------------------------------------------------------------------------
  1. Introduction
  2. Package Content
    2.1 Software for Target
    2.2 Development Environment
    2.3 Emulator
  3. Release Files
    3.1 T-Kernel Source Code Package [*srcpkg*]
    3.2 Target Related Information
    3.3 Development Environment
    3.4 emulator
  4. Work Flow
    4.1 Installation of Development Environment 
    4.2 Expanding T-Kernel Source Code Package
    4.3 Running T-Kernel on an emulator
    4.4 Building T-Monitor and Writing in ROM of Target Machine
    4.5 Building, Execution, and Debugging of T-Kernel 
    4.6 Building Device Driver 
    4.7 Building, Execution, and Debugging of Applications 
    4.8 Burning Data into ROM

------------------------------------------------
1. Introduction
------------------------------------------------

This document explains the procedure for building T-Monitor, T-Kernel
2.0 and the device driver for T-Engine Reference Board (tef_em1d), as
well as applications that run on them, using the development
environment of Eclipse or Linux command line, and for the execution
and debugging of the applications on the target machine.

The package of the T-Kernel source code (tkernel_source.tar.gz) in
addition to the package (this package) offered with this document is
necessary to build the system that uses T-Kernel. The package of 
the T-Kernel source code is registered in the T-Kernel
traceability service of T-Engine Forum, and can be obtained there.
Moreover, the package of the T-Kernel source code is governed
by T-License 2.0.

The content of this package and the content of the package of the
T-Kernel source code are explained below. "[*srcpkg*]" shows the
content included in the package of the T-Kernel source code.

--------------------------------------------------------------
2. Package Content
--------------------------------------------------------------

------------------------------------------------
2.1 Software for Target
------------------------------------------------

The source code and the build environment of the following software
are included.

    - T-Kernel 2.0      [*srcpkg*]
    - T-Monitor         [*srcpkg*]
    - Device Driver     [*srcpkg*]
        - Clock (RTC)
        - Serial console
        - Touch panel and pushbutton switch
        - LCD
        - System disk (for microSD)

* For a part of the device driver of the system disk (for microSD),
  the object code, instead of the source code, is included.

------------------------------------------------
2.2 Development Environment
------------------------------------------------

The following software is included. 

    - GNU development environment for Eclipse + Cygwin (Runs on Windows)
    - GNU development environment for Linux command line

------------------------------------------------
2.3 Emulator
------------------------------------------------

The following software is included. 

    - emulator (QEMU-tef_em1d) for the T-Engine reference board (tef_em1d)

Refer to readme_en.txt, "Instruction Manual for the emulator (QEMU-tef_em1d),"
for details.

------------------------------------------------
3. Release Files
------------------------------------------------

--------------------------------------------------
3.1 T-Kernel Source Code Package [*srcpkg*]
--------------------------------------------------

(1) Manuals 

  ReadMe_en.txt                 release manual (this document)
  tk2-dist-ucode.png            Distribution ucode
  TEF000-215-120911.pdf         T-License 2.0

  srcpkg/doc/en/ 
    tkernel.txt                 T-Kernel 2.0 Source Code Manual
    tmonitor.txt                T-Monitor Build Manual (for tef_em1d)
    driver.txt                  Device Driver Build Manual (for tef_em1d)
    impl-tef_em1d.txt           tef_em1d Implementation Specification
    gcc_setup_guide_linux.txt   GNU Development Environment and Installation Procedure (Linux)
    gcc_setup_guide_cygwin.txt  GNU Development Environment and Installation Procedure (Cygwin)
    eclipse_setup_guide.txt     Eclipse Development Environment and Installation Procedure 
    eclipse_guide.txt           T-Kernel Build Manual Using Eclipse (for tef_em1d) 

(2) Software

  srcpkg/ 
    tkernel_source.tar.gz       Source code package
                                (T-Kernel, T-Monitor, and the device driver)

(3) Binary for Execution

  srcpkg/bin/ 
    tmonitor.mot                T-Monitor ROM image
    rominfo.mot                 ROM image of ROM information (ROMInfo)
    kernel-rom.mot              T-Kernel ROM image

  * kernel-rom.mot is a ROM image of T-Kernel 2.0 built without device
    drivers.

--------------------------------------------------
3.2 Target Related Information
--------------------------------------------------

  hardware/tef_em1d/doc/en/
    TE_ReferenceBoard_HardwareManual.pdf
                                Reference Board Hardware Manual

  * The currently attached PDF is the original Japanese version.
    Translated English version will be available soon.

    target.txt                  Transfer to Target Machine and Operation Check Procedure (for tef_em1d)

  hardware/tef_em1d/ice/
    JETARM.CFG                  ICE Configuration File for PARTNER-Jet

--------------------------------------------------
3.3 Development Environment
--------------------------------------------------

  develop/ 
    te.Linux-i686.arm_2.1.0.3.tar.gz    GNU development environment and CPU dependent part (Linux)
    te.Linux-i686.common.13.tar.gz      GNU development environment and common part (Linux)
    
    cygwin_d-1.7.7-1.zip                Cygwin installation package 
    
    te.Cygwin-i686.arm_2.1.0.3.tar.gz   GNU development environment and CPU dependent part (Cygwin)
    te.Cygwin-i686.common.13.tar.gz     GNU development environment and common part (Cygwin)
    
    eclipse-cpp-indigo-SR2-incubation-win32.zip
                                        Eclipse+CDT package
    pleiades_1.3.4.zip                  Eclipse Japanese localization plug-in
    org.t_engine.te.1.1.0.zip           Eclipse T-Kernel common plug-in
    org.t_engine.tl.tef_em1d.1.1.0.zip  Eclipse tef_em1d plug-in

  [About GNU tools]

  - The original version of GNU tools, such as gcc and gdb, can be obtained
    from http://www.gnu.org/software/. The GNU tools used for this development
    environment are generated after patches are applied partly to the original
    version. How to obtain patches will be notified separately.

  - The license of GNU tools is GPL. See http://www.gnu.org/licenses/gpl.html
    for details.

  [About Cygwin]

  - Cygwin can be obtained from http://www.cygwin.com/. Cygwin
    included in this package is the version whose operation is checked
    and has been obtained from the URL above.

  - The license of Cygwin is GPL. See http://cygwin.com/license.html for details.

  [About Eclipse]

  - Eclipse can be obtained from http://www.eclipse.org/. Eclipse
    included in this package is the version whose operation is checked
    and has been obtained from the URL above.  

  - The license called Eclipse Public License (EPL) is applied to
    Eclipse. See http://www.eclipse.org/legal/epl-v10.html for
    details.

  - For Eclipse Japanese localization plugin, Eclipse Public License(EPL), 
    Mozilla Public License(MPL), Lesser General Public License(LGPL), 
    and Apache License are applied. See http://mergedoc.sourceforge.jp/index.
    html#/pleiades.html for details.

--------------------------------------------------
3.4 emulator
--------------------------------------------------

  emulator/                             Emulator Related Files

------------------------------------------------
4. Work Flow
------------------------------------------------

In this section, the overall work flow from the build of each program
to the execution and debugging of the program on the target machine is
explained in order.

------------------------------------------------
4.1 Installation of Development Environment
------------------------------------------------

Install either of the following development environments on the host
PC for development.

(1) When developing using Eclipse 

Eclipse can be used on Windows. Install Eclipse after having installed
Cygwin first, since Cygwin is necessary for development by Eclipse.
Install the development environment referring to the following manuals
in this order.

    gcc_setup_guide_cygwin.txt  GNU Development Environment and Installation Procedure (Cygwin)
    eclipse_setup_guide.txt     Eclipse Development Environment and Installation Procedure 

Refer to eclipse_guide.txt, "T-Kernel Build Manual Using Eclipse," for
the use of Eclipse. Moreover, read through the explanations of
"3. Program Build Method" and "3.1 General Build Method."

(2) When developing in Linux command line

Install the development environment referring to
gcc_setup_guide_linux.txt, "GNU Development Environment and
Installation Procedure (Linux)."

------------------------------------------------
4.2 Expanding T-Kernel Source Code Package
------------------------------------------------

Expand the package of T-Kernel source code (tkernel_source.tar.gz)
under /usr/local/tef_em1d (or c:\cygwin\usr\local\tef_em1d in case
Eclipse and Cygwin are used.)

Do the work up to "2. Starting Up Eclipse and Setting Workspace" in
eclipse_guide.txt, "T-Kernel Build Manual Using Eclipse," in case of 
development using Eclipse.

--------------------------------------------------
4.3 Running T-Kernel on an emulator
--------------------------------------------------

In this distribution, an emulator that emulates the operation of T-Engine
reference board (tef_em1d) is included.
Using this emulator, you can develop an application for T-Kernel 2.0 without
the target reference board on a Windows PC.

Please install the emulator, and check its operation, and
configure Eclipse accordingly.

See the following document and follow the steps in the named sections below.
"Instruction Manual for the emulator (QEMU-tef_em1d) for T-Engine reference
board (tef_em1d)"
emulator/tef_em1d/readme_en.txt

    1. Installing QEMU-tef_em1d
    2. Executing QEMU-tef_em1d
    4. Debugging on QEMU-tef_em1d (Eclipse)

-----------------------------------------------------------------
4.4 Building T-Monitor and Writing in ROM of Target Machine
-----------------------------------------------------------------

Programs including T-Kernel can be transferred to the target machine
and executed by using T-Monitor on the Flash ROM of the target
machine even if ICE is not used.

Therefore, first of all, it is necessary to write T-Monitor into the
Flash ROM of the target machine.

When T-Monitor is compiled from the source code and built, generate
tmonitor.mot, which is a ROM image of a T-Monitor object, referring to
tmonitor.txt, "T-Monitor Build Manual."

Moreover, tmonitor.mot that has already been built for T-Engine
Reference Board (tef_em1d) is included in the package of T-Kernel
source code. This can be used from the beginning instead.

Refer to target.txt, "Transfer to Target Machine and Operation Check
Procedure (for tef_em1d)," for the procedure for writing tmonitor.mot
into the target machine. It is necessary to use ICE to write T-Monitor
into Flash ROM for the first time if T-Monitor is not
written into Flash ROM.

---------------------------------------------------------
4.5 Building, Execution, and Debugging of T-Kernel
---------------------------------------------------------

Now, build the main body of T-Kernel, transfer it to the target
machine, execute and debug it.

When Eclipse is used, refer to "3.2 Build of Libraries," "3.3 Build of
Kernel for Loading RAM," "3.4 Build of Setup Information File for
Loading RAM" and "4. Execution and Debugging" in eclipse_guide.txt,
"T-Kernel Build Manual Using Eclipse," for the procedure. In
this case, build T-Kernel that is to be loaded on RAM 
for debugging by Eclipse. In case the build of T-Monitor
had previously been performed, "3.2 Build of Libraries" is not
necessary as it should have been completed.

When Linux command line is used, refer to "2. Build of
T-Kernel and Operation Check on Target Machine" in tkernel.txt,
"T-Kernel 2.0 Source Code Manual," for the procedure. In this case,
build and execute T-Kernel written into ROM.

-----------------------------------------
4.6 Building Device Driver
-----------------------------------------

The device drivers are linked with T-Kernel and become one object at
the time of execution. Therefore, if you execute device drivers
together with T-Kernel, first create a relocatable object of the
device driver for each device, then link these objects with T-Kernel,
and create the object code for execution.

Refer to "2. Device Driver Build Method" in driver.txt,
"Device Driver Build Manual," for specific procedures.

-------------------------------------------------------------
4.7 Building, Execution, and Debugging of Applications
-------------------------------------------------------------

User applications are also linked with T-Kernel, and become one object
at the time of execution. Generally, application programs are
described as modification or addition to usermain programs in T-Kernel
source code. Refer to "4.4 Application-dependent Part of Kernel
Directory" in tkernel.txt, "T-Kernel 2.0 Source Code Manual," for
details.

Use usermain_drv instead of usermain when the device drivers are
executed together. Refer to "2.8 Build of T-Kernel Main Body Including
Device Driver" in driver.txt, "Device Driver Build Manual."

The procedure for the execution and the debugging of T-Kernel built
with applications and device drivers is the same as that for T-Kernel
only (with no device driver or application). Refer to the above "4.4
Building, Execution, and Debugging of T-Kernel."

------------------------------------------------------------- 
4.8 Burning Data into ROM
------------------------------------------------------------- 

When the object code of T-Kernel linked with an application or device
drivers is burned into ROM after debugging using Eclipse is finished,
it is necessary to change the set up information file (config) written
into Flash ROM. Refer to "5. Build of Program for Writing into ROM" in
eclipse_guide.txt, "T-Kernel Build Manual Using Eclipse," for specific
procedures.

[EOF]
