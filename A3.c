/* Assignment 3*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>/*The file system header is the header required for writing
		       device drivers */
#include <linux/kdev_t.h>
#include <linux/cdev.h>/* Represent Char Devices Internally */
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>



#define DEVNAME "pci_blink"
#define DEVCNT 1


static long exam = 1;

struct class *pci_blink_class; 


/* Module Param */
char pci_blink_name[] = "pci_blink";
module_param(exam,long, S_IRUSR | S_IWUSR) ;

/* The module_param() macro takes 3 arguments: the name of the variable, 
 * its type and permissions for the corresponding file in sysfs. 
 * Integer types can be signed as usual or unsigned. */

/* this doesn't appear in /sys/module */
static long exam_nosysfs = 40;
module_param(exam_nosysfs, long, 0);


/* A kernel module that creates and
 * registers a single character device (struct cdev)*/

/* You will want to embed the cdev structure within a device specific
 * structure*/

static struct mydev_dev
{

	struct cdev cdev; /* Char device structure */

	long long syscall_val;/* Used by the user */

	dev_t pci_blink_node;

	long long led_init_val;

}mydev;


static struct mypci{

	struct pci_dev *pdev;
	void *hw_addr;

} mypci;



/*this shows up under /sys/modules/A3/parameters */

/* These symbolic constants are defined for the file mode 
 * bits that control access permission for the file: */
/* Read permission bit for the owner of the file */
/* Write permission bit for the owner of the file */




/* Open is the method called every time someone opens
 * your device's file. Device opening will always be successful in cases where
 * this method is not defined. You usually use this method to perform device
 * and data structure initialization, and return a negative error code if
 * something goes wrong or 0. The prototype of the open method is defined as :
 * int (*open)(struct inode *inode, struct file *flip);*/

static int pci_blink_open(struct inode *inode, struct file *file)
{

	struct mydev_dev *mydev = NULL;
	/* Get the per - device structure that contains this cdev */
	mydev = container_of(inode->i_cdev, struct mydev_dev, cdev);

	/* Initialize */

	mydev -> syscall_val = 40;

	/* Easy access to mydev_dev from rest of the entry points */
	file->private_data = mydev;

	printk(KERN_INFO "Success! Opened ! \n");
	return 0;


	/* For each open performed on your char device , the callback function will be
	 * given a struct inode as a parameter, which is the kernel lower-level
	 * representation of the file. The struct inode structure has a field named
	 * i_cdev, which points to the cdev we have allocated in the init function */

}

static ssize_t pci_blink_read(struct file *file, char __user *buf, size_t len, loff_t
		*offset)
{
	/* - *buf is the buffer we receive from the user space
	 * - len is the size of the requested transfer (size of the user buffer)
	 * - *offset indicates the start position from which the data should be read in
	 * the file*/

	i	/* Get a local kernel buffer set aside */
	int ret;

	if (*offset >= sizeof(int32_t)) return 0; /* */


	/*  Copy the data into the user space buffer and return an error on failure : */

	mydev.led_init_val = readl (mypci.hw_addr + 0xE00);

	if (copy_to_user (buf,&mydev.syscall_val, sizeof(int32_t)))
	{
		ret = -EFAULT;
		goto out;
	}

	/* Make sure our user wasn't bad...*/
	if (!buf)
	{
		ret = EINVAL;
		goto out;
	}

	ret = sizeof(long);
	*offset += len;

	/* Success */

	printk(KERN_INFO "User got from us %lld \n", mydev.syscall_val);

out:
	return ret;

}

static ssize_t pci_blink_write(struct file *file, const char __user *buf,size_t len,
		loff_t *offset)
{
	/* The write method is used to send data to the device,
	 * whenever a user app calls the write function on the device's file, the
	 * kernel implementation is called. Its prototype is as follows:
	 * ssize_t(*write)(struct file *flip, const char _user *buf, size_t count,
	 * loff_t *pos);
	 *
	 * - The return value is the number of bytes(size) written
	 * - *buf represents the data buffer coming from the user space
	 * - count is the size of the requested transfer
	 * - *pos indicates the start position from which the data should be written in
	 * the file.
	 *
	 *Suggested steps:
	 * 1 . Check for bad or invalid requests coming from the user space.
	 * 2 . Adjust count for the remaining bytes in order to not go beyond the file
	 *   size.
	 * 3 . Find the location from which you will start to write.
	 * 4 . Copy data from the user space and write it into the appropriate kernel
	 *   space.
	 * 5 . Write to the physical device and return an error on failure. */

	/* Have local kernel memory ready */
	int *kern_buf;
	int ret;

	/* Make sure our user isn't bad... */
	if (!buf)
	{
		ret = -EINVAL;
		goto out;
	}

	/* Get some memory to copy into... */
	kern_buf = kmalloc (len, GFP_KERNEL);

	/* ...and make sure it is good to go */

	if (!kern_buf)
	{
		ret = -ENOMEM;
		goto out;

	}

	/* Copy from the user-provided buffer */
	if (copy_from_user(&mydev.syscall_val, buf, len))
	{
		ret = -EFAULT;
		goto mem_out;

	}

	ret = len;

	/* print what userspace gave us */

	exam = mydev.syscall_val;
	printk (KERN_INFO "Userspace wrote \"%ld\" to us \n", exam);
	writel(exam,mypci.hw_addr + 0xE00);


mem_out:
	kfree (kern_buf);
out:
	return ret;

}

int  pci_blink_release (struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations mydev_fops = {

	/* C99 way of assigning to elements of a structure,
	 * 	you should use this syntax in case someone wants to
	 * 	port your driver */

	.owner = THIS_MODULE,
	.open  = pci_blink_open,
	.read  = pci_blink_read,
	.write = pci_blink_write,
	.release = pci_blink_release,
};


static const struct pci_device_id pci_blink_table[] = {

	{ PCI_DEVICE(0x8086,0x100e) },
	{},

};

/* Enables the PCI device
 * Discovers resource information such as I/O base addresses
 * Allocates and populates a networking data structure associated with this
 * device
 * Registers itself with the kernel networking layer
 * The PCI subsystem calls probe() with two arguments
 *
 * 1. A pointer to pci_dev, the data structure that describes this PCI device
 * 2. A pointer to pci_device_id, the entry in the driver's pci_device_id table
 * that matches the information found in the configuration space.
 *
 * Use this method only when you are sure that the device exists
 */



static int pci_blink_probe (struct pci_dev *pdev, const struct
		pci_device_id *ent)
{
	resource_size_t mmio_start, mmio_len;
	unsigned long barmask;
	printk (KERN_INFO "Blink Driver PCI Probe called \n");

	// get BAR mask

	barmask = pci_select_bars (pdev, IORESOURCE_MEM);

	printk (KERN_INFO "barmask %lx", barmask);

	// reserve BAR areas
	if (pci_request_selected_regions (pdev, barmask, pci_blink_name))
	{
		printk(KERN_ERR "request selected regions failed \n");

		goto unregister_selected_regions;
	}

	mmio_start = pci_resource_start(pdev,0);
	mmio_len = pci_resource_len(pdev,0);
	printk(KERN_INFO "mmio start: %lx", (unsigned long) mmio_start);
	printk(KERN_INFO "mmio len: %lx ",(unsigned long) mmio_len);

	if( ! (mypci.hw_addr = ioremap (mmio_start, mmio_len)))
	{
		printk(KERN_ERR "ioremap failed \n");
		goto unregister_ioremap;
	}



	mydev.led_init_val = readl (mypci.hw_addr + 0xE00);
	printk(KERN_INFO "initial val is %llx \n",mydev.led_init_val);







	/* File operations for our device */

	return 0;

unregister_ioremap:
	iounmap(mypci.hw_addr);

unregister_selected_regions:

	pci_release_selected_regions(pdev,pci_select_bars(pdev,IORESOURCE_MEM));

}

static void pci_blink_remove(struct pci_dev *pdev)
{

	printk(KERN_ERR "PCI removed \n" );
	iounmap (mypci.hw_addr);
	pci_release_selected_regions (pdev, pci_select_bars (pdev, IORESOURCE_MEM));

}

/* struct pci_driver if defined in include/linux/pci.h */

static struct pci_driver pci_blink = {

	.name = "pci_blink_driver",
	.id_table = pci_blink_table,
	.probe = pci_blink_probe,
	.remove = pci_blink_remove,

	/* suspend() and resume() methods that implement power management
	 * are not used by this driver */

};

/* Driver initialization */
static int __init pci_blink_init(void)
{

	/* Request Dynamic allocation of a device major number */

	printk(KERN_INFO "pci module loading.. val  =%lx\n",exam);

	if(alloc_chrdev_region(&mydev.pci_blink_node, 0, DEVCNT, DEVNAME) < 0)
	{
		printk(KERN_ERR "alloc_chrdev_region() failed! \n");

		return -1;
	}

	/* Populate sysfs entries */

	pci_blink_class = class_create(THIS_MODULE, DEVNAME);

	/* Send uevents to udev , so it'll create /dev nodes */
	device_create(pci_blink_class,NULL,mydev.pci_blink_node, NULL, DEVNAME);

	/* Initialize the char device and add it to the kernel */

	/* Connect the file operations with the cdev */
	cdev_init (&mydev.cdev, &mydev_fops);

	mydev.cdev.owner = THIS_MODULE;/* has an owner field that should be set to
					  "THIS_MODULE" */

	/* Connect the major /minor number to the cdev */

	if (cdev_add (&mydev.cdev,mydev.pci_blink_node,DEVCNT))
	{
		printk (KERN_ERR "cdev_add() failed ! \n");

		/* Clean up chrdev allocation */

		unregister_chrdev_region(mydev.pci_blink_node, DEVCNT);

		return -1;
	}

	printk (KERN_INFO "Allocated %d devices at major: %d \n", DEVCNT,
			MAJOR(mydev.pci_blink_node));


	if (cdev_add(&mydev.cdev,mydev.pci_blink_node,DEVCNT))
	{
		printk(KERN_ERR "cdev_add() failed \n ");
		unregister_chrdev_region(mydev.pci_blink_node, DEVCNT);

	}

	// register as a pci driver

	if (pci_register_driver (&pci_blink))
	{
		printk(KERN_ERR "pci_register _driver failed \n");
		goto unreg_pci_driver;
	}

	// success


	return 0;


unreg_pci_driver : //clean up pci driver registration
	pci_unregister_driver(&pci_blink);


}
//---------------------------------------

static void __exit pci_blink_exit(void)
{
	/*destroy the cdev */
	cdev_del(&mydev.cdev);

	/* clean up the devices */
	unregister_chrdev_region(MAJOR(mydev.pci_blink_node), 1);

	/* Destroy pci_class */
	class_destroy(pci_blink_class);

	printk(KERN_INFO "pci module loading... val =%lx\n",exam);
	printk(KERN_INFO "pci device unloaded ! \n");

}


MODULE_AUTHOR(" NIK NIKOLOV ");
MODULE_LICENSE("GPL");
MODULE_VERSION("2100");

module_init(pci_blink_init);
module_exit(pci_blink_exit);



