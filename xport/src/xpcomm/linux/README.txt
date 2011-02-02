Please send questions, comments and bugs to support@charmedlabs.com

Thanks to Kiran Nair for providing this Linux port!!

Below are some notes he has provided:

This version of the port uses the linux ppdev module
to access the parallel port.

If you already don't have a entry /dev/parportN (N for the  parallel port number)
Then you need to create one using teh following command

mknod /dev/parport0 c 99 0

My Mandrake 10.0 did not have one :-(

Later you need to edit /etc/modules.conf to add an alias as shown in the below line
alias /dev/parport0 ppdev

reboot???

You should get something like this later on

root@localhost kiran]# lsmod | grep "ppdev"
ppdev                   9408  0
parport                38952  4 ppdev,ppa,parport_pc,imm

Now xpcomm should work fine both as a user and root
