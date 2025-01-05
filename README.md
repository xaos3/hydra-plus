Hydra+ is a small interpreter language that can be compiled for Windows and Linux platforms.
As for now (05-01-2025) this repository hosts the Linux version of the source code , but when i have the 
time i will upload the windows version too. If you want to test the windows version , you can download a precompiled 
Hydra+ Interpreter at the deus-ex.gr/varius/hydra+.zip

Below i will explain the basic operation of the Hydra+ , why i create it and how you can compile the source code for Linux 
that this repository hosts (NOTE : The source code has precompiled some third party libraries for convinience , that limits the compile to x86-64bit systems that are Ubuntu based.
See the relevant informations in the compile section later in the text).

-- What is Hydra+ ? 
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
Hydra+ code is pure C. I dislike "smart" code, so my code is the simplest (most of the time) C code that can you find. No fancy syntax, no exotic things.
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

If are you thinking this is half the code of Hydra+ , well maybe xD.

-- Where Hydra+ can be used
I will mention the uses that I have (and thus created) for Hydra+ :
- Creating data exporting scripts from databases
- Creating small web servers for serving localhost javascript applications in web browsers
- Creating web services and APIS in servers.
- Creating complex webservers for server side applications
- Creating importers for databases from raw text
- Creating middlewares from one platform to another (ERP application to Eshop)

















