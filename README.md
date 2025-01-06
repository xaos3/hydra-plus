Hydra+ is a small interpreter language that can be compiled for Windows and Linux platforms.
As for now (05-01-2025) this repository hosts the Linux version of the source code , but when i have the 
time i will upload the windows version too. If you want to test the windows version , you can download a precompiled 
Hydra+ Interpreter at the deus-ex.gr/varius/hydra+.zip

Below i will explain the basic operation of the Hydra+ , why i create it and how you can compile the source code for Linux 
that this repository hosts (NOTE : The source code has precompiled some third party libraries for convinience , that limits the compile to x86-64bit systems that are Ubuntu based.
See the relevant informations in the compile section later in the text).

## What is Hydra+ ? 

Hydra+ is a single executable (see later in the text about dependencies) that packs a loader and interpreter for the Hydra+ language syntax.

-- Why Hydra+ was created ?

I will admit that some times my brain make things hard for me. I have the need to control the tools that i use (thankfully with limits), and
i truly hate the bloat. The idea about Hydra+ came to me when I had to create a very small program to manipulate and export some data in a Client machine, and i realise
that i trully hate the bloat of PHP , Python (i hate Python for alot more reasons) and other languages that needs so many libraries to do 
a very simple job. Another problem was that while i enjoy to write in Free Pascal or C (Hydra+ is created in C after all) when in the client , if i need to make some changes 
to the source code i have to do it in a machine with all the tools chain present. I hate this too. I would write in power shell in windows or 
in bash in Linux BUT... why the script code is so ugly? It's only me that see this? Anyhow I decided to create a language that will have the capabilities that
I use the most in my everyday work life (so life in general !).

-- How Hydra+ was created

Hydra+ code is pure C. I dislike "smart" code, so my code is the simplest (most of the time) C code that you can find. No fancy syntax, no exotic things.
The source code is create by me in its entirety, except the third party libraries for :
SSL support (Wolf SSL) 
Threads support (cthreads)
base64 support (by joseph werle),
processes support (reproc)
zip/unzip support (zipcuba),
image manipulation support (stbimage etc)
SQLite native support (amagalmation source files),
MariaDB support (mariadb connector)
ODBC support via the native library of the compiler

If you are thinking this is half the code of Hydra+ , well maybe xD.

-- Where Hydra+ can be used

I will mention the uses that I have (and thus created) for Hydra+ :
- Creating data exporting scripts from databases
- Creating small web servers for serving localhost javascript applications in web browsers
- Creating web services and APIS in servers.
- Creating complex webservers for server side applications
- Creating importers for databases from raw text
- Creating middlewares from one platform to another (ERP application to Eshop)

-- Hydra+ advantages

- Very small executable (~3MB), no third party dependencies when compiled (except Linux, see later in the text)
- Native support of UTF8 strings. (and the only ones supported xD) 
- Native support of MariaDB
- Native support of ODBC connections (tested for SQL SERVER)
- Native support for threads via asynchronous functions
- Native support of SSL and plain TCP connections, client and server
- Few language keywords , easy to learn
- Can obfuscate it's code with an internal key, that make the code incomprehensible to the human eye (hydra_loader.h line 61 change the key!!)
- Handsome creator (this is a joke)

-- Hydra+ disadvantages

- To support new functions and operations the source code must change and the Hydra+ must recompiled.
  I do not support new functions via libraries. I may do it in the future but it will defeat the purpose of the
  single and small executable. I'm in the fence about it.
- Limited support. I'm the only one that write code for this language. I add functions add new features and fix bugs
  when I need them for my work. So if a new feature is needed but i do not is very likely that I never will implement it.
- Hidden bugs or memory leaks. I repeat that I'm the only one writing code for Hydra+ and I have hand written all the structures , lists ,
  logic , loader, interpreter, parser etc. This means that is very possible that catastrophic bugs are lurking. I have test Hydra+ for at least 3
  months in production, in small applications and i have fix a lot of memory leaks and bugs (mostly for the asynchronous operations). But
  I'm not confident that the worst has passed!
- Execute speed (maybe?). I did not have made extensive benchmarks (yet , no time!) , but the interpretation (the code is stored in a kind of abstract tree)
  is a lot slower than PHP's (when tested with 10.000.000 calculation with no input/output). Of course in real operation , when you write/read in disk or send data via sockets
  the performance (in some limited test I have done) is similar with PHP, because the actual time consuming operations are these and not
  the code interpretation. If anyone do any benchmark please inform me! 
- Hydra+ is very verbose, as it has limited keywords and none of the shortcut syntaxs, like for example the "foreach" of PHP. 
- Hydra+ imposes some restrictions in scipts like that the include directive can be present only in the top of the main script and nowere else.
  Some of these restrictions (that you will find in the documentation) maybe seems weird and very easy to implement. They trully are. BUT some things tha I have do is for forcing me to write cleaner code! 
- A lot of other things to consider!

-- Writing Hydra+ scripts

Please read all the documentation that you will find in this repository. It does not only have the syntax for the language but explain the intricacies
of the internal operation and when (VERY IMPORTANT) you have to free manually the memory of your objects.
The documentation has all the info that are needed for someone to write scripts, but I will provide gradually examples of scripts when I have time, as I
understand that examples are always better than documentation. 

-- Compilation

To compile Hydra+ for Linux (later I will upload the windows version for Visual Studio) you have to know
at least the basics of Linux.
The linux directory includes a precompiled library of the "wolfssl" and for "reproc" so that the compilation will be faster.
NOTE : The precompiled libraries are compiled for Ubuntu Linux for x32-64bit platforms. If you want to change 
target platform you have to recompile the libraries and replace them. The libraries are  in :

/includes/thirdparty/wolfssl
and
/includes/thirdparty/reproc (i do not remember why I precompiled this as it is a very small library xD)

First download the linux folder (I assume as a zip file) and extracted it in a directory , I will assume that the directory will be named comp_hydra.

***
If you plan to use the obfuscation ability of Hydra+, then go to the /includes/hydra_loader.h in the line 61 and change the obfuscation key to something else!  
***

In the comp_hydra directory you will find a script with the name "compile_hydra.sh". Give the executable privilege to the file with chmod.
Run the script. 
- The script will prompt you to download the build-essential (gcc and libraries) if you do not have them in your system
press y.
- The script will prompt you to download the ODBC driver and libraries. If you do not have them in your system and want to use odbc connections
  press y.
- The script will prompt you to download MariaDB client libraries, if you do not have them in your system and want to use the MariaDB
  press y.
- The script will prompt you to start the compilation. Press y.

Wait for the compilation, if everything is correct the script will emmit an "Operation completed" message.

Now you can find the binary file of Hydra+ in the comp_hydra/hydra+/bin/
the name of the binary will be hydra+

Now you can use it as :  hydra+ myscript.hydra


If you have any question feel free to contact me. I may not respond immediately as I have too much work and very little sleep! 







