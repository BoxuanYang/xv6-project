# COMP3300/COMP6330: Instructions for setting up the lab virtual machine (VM) 

For the labs in this course, we will use a number of tools and emulation software to do the exercises and assignments. For your convenience we have created a VM image, which is based on Ubuntu 22.04.2 (server edition) for Intel x86-64 architecture. Below are two options to access this VM image: 

-	VirtualBox VM. This is the recommended VM to use outside the lab sessions. You will need to install the [VirtualBox](https://www.virtualbox.org) software in your computer to run this VM. 

-	UTM VM. This is similar to the VirtualBox VM, but is specifically set up to run on Mac computers that use Apple Silicon CPUs. You need to have the [UTM app](https://getutm.app) installed in your Mac to run this VM.



# Lab VM images 

To install the lab VM, download either the VirtualBox VM image or the UTM VM image. 

- VirtualBox VM Image: [comp3300-2025.ova](https://anu365-my.sharepoint.com/:u:/g/personal/u4301469_anu_edu_au/EbR_4ZF-vVlKvEo4-H9m11EBBk3emYPb26p86OW6DDWEaw?e=F6EKAp). To install the image, simply double-click the OVA file.

- UTM VM Image: [comp3300-2025.utm.zip](https://anu365-my.sharepoint.com/:u:/g/personal/u4301469_anu_edu_au/EQewkAoXwStFg4_5ZlmEv-oBi3BuVLx_0I3VskKbVZGoOw?e=onfoti). Unzip the the ZIP file, and double-click the uncompressed app to launch it. 

# Connecting to the VM

The username for the VM is `osilab` and the default password is `ANU_comp3300` (make sure you change this after login). 

Although both VirtualBox and UTM provide virtual consoles you can log into, it is best to connect to the VM using an `ssh` client. The VM runs an SSH server. Both VMs forward the SSH port (22) to port number 56789. To connect to the VM via ssh from the host machine, simply run the following command line:

```
ssh -p 56789 osilab@localhost
```


# Transferring data between the host system and the lab VM

For most of the tasks you will likely use the CSS Teaching gitlab, so you can use that to save your work you’ve done inside the VM. 

In case you also want to transfer files between the host OS running in your computer and the VM, you can use a number of tools, such as rsync, scp or sftp. We show here how to use sftp – for other commands, please consult their respective manuals. 

The sftp program is supported natively in many operating systems, such as Windows, Linux and Mac OS. The command you need is `sftp`. The command to connect to the lab VM using sftp is very similar to ssh: 

```
sftp -P 56789  osilab@localhost
```

Note that the option for specifying the port number uses an upper-case ‘P’, instead of the lower-case ‘p’ in ssh. 

Once you are connected to the lab VM, you will be presented with a `sftp>` prompt, in which you can type commands. Two basic useful commands are get (to download files from the VM) and put (to upload files to the VM). Here are the two commands you need to upload and download files to the VM (assuming the file you want to download/upload is called `myfile.txt`): 

```
sftp> get myfile.txt 
```
This command will retrieve the file myfile.txt from the current directory on the VM (by default it is the home directory of the logged in user).  

```
sftp> put myfile.txt
```
This command  will upload the file “myfile.txt” in the current directory on the VM. 

# Using VSCode with the lab VM

If you use VS Code editor, you can connect directly to the VM using Microsoft [Remote-SSH extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-ssh). 
See 

[https://code.visualstudio.com/docs/remote/ssh](https://code.visualstudio.com/docs/remote/ssh)

for detailed instructions to set this up. 


