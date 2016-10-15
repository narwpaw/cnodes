#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/bug.h>


#define BIT_RATE					7


#define NODES 				35
#define IN_REGISTERS 		16
#define OUT_REGISTERS 		16
#define SPI_CHANNEL			1
#define BUILD_MODULE   1
#define RFM12B_DRV_NAME     "cnodes"

#include "rfm12b_config.h"
#include "rfm_data_struct.h"
#include "rfm.h"

static struct rfm12_data rfm12;


unsigned short pCharToWord(const unsigned char *ptr)
{
	union{
		unsigned char  u8;
		unsigned short u16;
	}val;

	val.u8=*ptr;
	return val.u16;
}




bool set_input_reg(const unsigned char node, const unsigned char *attr_name, const unsigned char *set)
{
	long reg;

	if (strstr (attr_name, "in_reg_")==0) return false;
	if (kstrtol(attr_name+7,0, &reg)) return false;

	if (reg>IN_REGISTERS)
		{
			printk(">>> Aborted. Wrong input nr %d\n", node);
			return 0;
		}

	rfm12.nodes[node].in_reg[reg]=pCharToWord(set);
	printk(">>> Set input register %ld = %d\n", reg, rfm12.nodes[node].in_reg[reg]);
	return true;
}

bool set_output_reg(const unsigned char node, const unsigned char *attr_name, const unsigned char *set)
{
	long reg;

	if (strstr (attr_name, "out_reg_")==0) return false;
	if (kstrtol(attr_name+8,0, &reg)) return false;

	if (reg>IN_REGISTERS)
		{
			printk(">>> Aborted. Wrong output nr %d\n", node);
			return 0;
		}

	rfm12.nodes[node].out_reg[reg]=pCharToWord(set);
	printk(">>> Set output register %ld = %d\n", reg, pCharToWord(set));
	return true;
}


bool set_node(const unsigned char *node_name, const unsigned char *attr_name, const unsigned char *set)
{
	long node;

	if (strstr (node_name, "node_")==0) return false;
	if(kstrtol(node_name+5,0, &node)) return false;

	if (node>NODES)
	{
		printk(">>> Aborted. Wrong node %ld\n", node);
		return 0;
	}

	printk(">>> Set node %ld\n", node);


	if (set_input_reg(node, attr_name, set)) return true;
	if (set_output_reg(node, attr_name, set)) return true;
	return true;
}











bool get_input_reg(const unsigned char node, const unsigned char *attr_name, unsigned char *get)
{
	long reg;
	unsigned short *reg_get;

	if (strstr (attr_name, "in_reg_")==0) return false;
	if (kstrtol(attr_name+7,0, &reg)) return false;

	if (reg>IN_REGISTERS)
		{
			printk(">>> Aborted. Wrong input register nr %d\n", node);
			return 0;
		}

	reg_get = (unsigned short*)get;

	*reg_get=rfm12.nodes[node].in_reg[reg];
	printk(">>> Get input register %ld = %d\n", reg, *reg_get);
	return true;
}


bool get_output_reg(const unsigned char node, const unsigned char *attr_name, unsigned char *get)
{
	long reg;
	unsigned short *reg_get;

	if (strstr (attr_name, "out_reg_")==0) return false;

	if (kstrtol(attr_name+7,0, &reg)) return false;

	if (reg>IN_REGISTERS)
		{
			printk(">>> Aborted. Wrong output register nr %d\n", node);
			return 0;
		}

	reg_get = (unsigned short*)get;
	*reg_get=rfm12.nodes[node].out_reg[reg];
	printk(">>> Get output register %ld = %d\n", reg, *reg_get);
	return true;
}




bool get_node(const unsigned char *node_name, const unsigned char *attr_name, unsigned char *get)
{
	long node;

	if (strstr (node_name, "node_")==0) return false;
	if (kstrtol(node_name+5,0, &node)) return false;

	if (node>NODES)
	{
		printk(">>> Aborted. Wrong node %ld\n", node);
		return 0;
	}

	printk(">>> Get node %ld\n", node);

	if (get_input_reg(node, attr_name, get)) return true;
	if (get_output_reg(node, attr_name, get)) return true;
	return true;
}




static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,  char *buf)
{

	printk(">>> b_show %s, %s \n", kobj->name, attr->attr.name);

	if (get_node(kobj->name, attr->attr.name, buf)) return 2;


//	int var;
//
//	if (strcmp(attr->attr.name, "baz") == 0)
//		var = baz;
//	else
//		var = bar;
//	return sprintf(buf, "%d\n", var);
	return 0;
}


static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, const size_t count)
{

	printk(">>> b_store %s, %s  - count: %d\n", kobj->name, attr->attr.name, count);

//	spi_write_reg(buf[0], buf[1]);

	if (set_node(kobj->name, attr->attr.name, buf))
	{
		printk(">>> sysfs_notify \n");
		sysfs_notify(kobj, NULL,  attr->attr.name);
		return count;
	}
	return 0;
}


int get_list(const unsigned char *name, char *buf)
{
	int size=0;
	int node=0;
	char i;
	char header[]="%-5s %10s %10s %16s %4s\n";
	char format[]="%-5d %10d %10d %16s 0x%04x\n";
	char status[17];
	status[16]=0;
	//if (strstr (name, "list")==0) return 0;
	//printk(">>> get_list : \s\n", name);

	size = sprintf(buf, "\n");
	size += sprintf(buf+size, header, "Node", "Id", "Signal", "Status", "In reg. 0");
	size += sprintf(buf+size, header, "=====", "=========", "=========", "===============", "=========");

	for (node = 1; node<NODES; node++)
	{

		for (i=0; i<16; i++)
		{
			status[i]=((rfm12.nodes[node].in_reg[0]>>i) & 0x1) +'0';
		}
		size += sprintf(buf+size, format, node, rfm12.nodes[node].id, 0, status, rfm12.nodes[node].in_reg[0]);
	}
	//sprintf(buf, "node_%d", i);





	return size;
}

static ssize_t info_show(struct kobject *kobj, struct kobj_attribute *attr,  char *buf)
{
	printk(">>> info_show \n");
	int res;

	res=get_list(kobj->name, buf);
	if (res) return res;

	return 6;
}




static struct kobj_attribute node_attributes[] =
		{
				__ATTR(in_reg_0,   0664, b_show, b_store),
				__ATTR(in_reg_1,   0664, b_show, b_store),
				__ATTR(in_reg_2,   0664, b_show, b_store),
				__ATTR(in_reg_3,   0664, b_show, b_store),
				__ATTR(in_reg_4,   0664, b_show, b_store),
				__ATTR(in_reg_5,   0664, b_show, b_store),
				__ATTR(in_reg_6,   0664, b_show, b_store),
				__ATTR(in_reg_7,   0664, b_show, b_store),
				__ATTR(in_reg_8,   0664, b_show, b_store),
				__ATTR(in_reg_9,   0664, b_show, b_store),
				__ATTR(in_reg_10,  0664, b_show, b_store),
				__ATTR(in_reg_11,  0664, b_show, b_store),
				__ATTR(in_reg_12,  0664, b_show, b_store),
				__ATTR(in_reg_13,  0664, b_show, b_store),
				__ATTR(in_reg_14,  0664, b_show, b_store),
				__ATTR(in_reg_15,  0664, b_show, b_store),
				__ATTR(out_reg_0,  0664, b_show, b_store),
				__ATTR(out_reg_1,  0664, b_show, b_store),
				__ATTR(out_reg_2,  0664, b_show, b_store),
				__ATTR(out_reg_3,  0664, b_show, b_store),
				__ATTR(out_reg_4,  0664, b_show, b_store),
				__ATTR(out_reg_5,  0664, b_show, b_store),
				__ATTR(out_reg_6,  0664, b_show, b_store),
				__ATTR(out_reg_7,  0664, b_show, b_store),
				__ATTR(out_reg_8,  0664, b_show, b_store),
				__ATTR(out_reg_9,  0664, b_show, b_store),
				__ATTR(out_reg_10, 0664, b_show, b_store),
				__ATTR(out_reg_11, 0664, b_show, b_store),
				__ATTR(out_reg_12, 0664, b_show, b_store),
				__ATTR(out_reg_13, 0664, b_show, b_store),
				__ATTR(out_reg_14, 0664, b_show, b_store),
				__ATTR(out_reg_15, 0664, b_show, b_store),
		};

static struct attribute *attrs[] = {
	&node_attributes[0].attr,
	&node_attributes[1].attr,
	&node_attributes[2].attr,
	&node_attributes[3].attr,
	&node_attributes[4].attr,
	&node_attributes[5].attr,
	&node_attributes[6].attr,
	&node_attributes[7].attr,
	&node_attributes[8].attr,
	&node_attributes[9].attr,
	&node_attributes[10].attr,
	&node_attributes[11].attr,
	&node_attributes[12].attr,
	&node_attributes[13].attr,
	&node_attributes[14].attr,
	&node_attributes[15].attr,
	&node_attributes[16].attr,
	&node_attributes[17].attr,
	&node_attributes[18].attr,
	&node_attributes[19].attr,
	&node_attributes[20].attr,
	&node_attributes[21].attr,
	&node_attributes[22].attr,
	&node_attributes[23].attr,
	&node_attributes[24].attr,
	&node_attributes[25].attr,
	&node_attributes[26].attr,
	&node_attributes[27].attr,
	&node_attributes[28].attr,
	&node_attributes[29].attr,
	&node_attributes[30].attr,
	&node_attributes[31].attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};






static struct kobj_attribute info_attribute = __ATTR(list, 0664, info_show, NULL);

static struct attribute *attrs_info[] = {
	&info_attribute.attr,
	NULL,
};

static struct attribute_group attr_info_group = {
	.attrs = attrs_info,
};

static struct kobject *root_kobj = NULL;
static struct kobject *node_kobj[NODES];
static struct kobject *info_kobj;




//static struct spi_device *spi_device;

static void spidevices_delete(struct spi_master *master, unsigned cs)
{
	struct device *dev;
	char str[32];

	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev), cs);

	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);
	if (dev) {
		printk(KERN_INFO RFM12B_DRV_NAME ": Deleting %s\n", str);
		device_del(dev);
	}
}


static uint spi_bus = 0;
static uint spi_cs = 1;
static uint spi_speed_hz = 2000000;
static uint spi_bits_per_word = 8;

static inline  int spi_init(void) {
    struct spi_board_info spi_device_info = {
        .modalias = RFM12B_DRV_NAME,
        .max_speed_hz = spi_speed_hz,
        .bus_num = spi_bus,
        .chip_select = spi_cs,
        .mode = 0,
    };

    struct spi_master *master;
    struct spi_device* spi_device;
    struct device* sdev;

    int ret;
    char buf[128];

    // get the master device, given SPI the bus number
    master = spi_busnum_to_master( spi_device_info.bus_num );
    if( !master )
        return -ENODEV;

    spidevices_delete(master, spi_device_info.chip_select);


    // create a new slave device, given the master and device info
    active_boards[0].spi_device = spi_new_device( master, &spi_device_info );
    if( !active_boards[0].spi_device )
        return -ENODEV;

    put_device(&master->dev);
       if (!active_boards[0].spi_device) {

          return -EPERM;
       }
    return ret;
}


static inline void spi_exit(void) {
    spi_unregister_device( active_boards[0].spi_device );
}



static int __init cnodes_init(void)
{
	int retval;
	int i;
	int err;
	//int ret;
	char str[10];
	char buf[5]={1,2,3,4,0};

	printk(">>>>>>> INIT2\n");
	spin_lock_init(&rfm12.lock);

	spi_init();

//	struct rfm12_data rfm12;
	rfm12.spi = active_boards[0].spi_device;
	rfm12.bit_rate= BIT_RATE;

	
	//tmp -------------------------------------
	rfm12.nodes[1].id=100005;
	rfm12.nodes[2].id=100078;
	rfm12.nodes[3].id=100245;
	rfm12.nodes[4].id=100244;
	rfm12.nodes[5].id=133005;
	rfm12.nodes[6].id=100003;
	rfm12.nodes[7].id=100149;
	rfm12.nodes[8].id=100058;

	rfm12.nodes[9].id=111111;
	rfm12.nodes[10].id=100299;
	rfm12.nodes[11].id=100002;
	rfm12.nodes[12].id=100261;
	rfm12.nodes[13].id=100151;
	rfm12.nodes[14].id=100000;
	rfm12.nodes[15].id=100442;
	rfm12.nodes[16].id=100412;
	rfm12.nodes[17].id=100443;
	rfm12.nodes[18].id=100148;
	rfm12.nodes[19].id=100149;
	rfm12.nodes[20].id=100838;


	rfm12.nodes[1].type=3;
	rfm12.nodes[2].type=3;
	rfm12.nodes[3].type=5;
	rfm12.nodes[4].type=5;
	rfm12.nodes[5].type=5;
	rfm12.nodes[6].type=3;
	rfm12.nodes[7].type=3;
	rfm12.nodes[8].type=3;
	rfm12.nodes[8].type=3;
	rfm12.nodes[10].type=3;
	rfm12.nodes[11].type=3;
	rfm12.nodes[12].type=3;
	rfm12.nodes[13].type=3;
	rfm12.nodes[14].type=3;
	rfm12.nodes[15].type=3;
	rfm12.nodes[16].type=3;
	rfm12.nodes[17].type=3;
	rfm12.nodes[18].type=3;
	rfm12.nodes[19].type=3;
	rfm12.nodes[20].type=3;
   // -------------------------------------------------


	//rfm12_start_receiving(&rfm12);
	//spi_write( active_boards[0].spi_device, buf, 4 );

	    printk(KERN_INFO "cnodes : init cnodes\n" );
	    root_kobj = kobject_create_and_add("cnodes", kernel_kobj);


	for (i=0; i<NODES; i++)
	{
		sprintf(str, "node_%d", i);
		node_kobj[i] = kobject_create_and_add(str, root_kobj);
		if (!node_kobj[i])
			return -ENOMEM;

		retval = sysfs_create_group(node_kobj[i], &attr_group);

		if (retval) kobject_put(node_kobj[i]);
	}


	info_kobj = kobject_create_and_add("info", root_kobj);
	retval = sysfs_create_group(info_kobj, &attr_info_group);
	if (retval) kobject_put(info_kobj);


	r_int_config();

	rfm12_setup(&rfm12);
	msleep(2);
	active_boards[0].irq_data =  &rfm12;
	rfm12_initRx(&rfm12);

	return retval;
}

static void __exit cnodes_exit(void)
{
	int i;

	r_int_release();
	spi_exit();

	for (i=0; i<NODES; i++)
		{
			kobject_put(node_kobj[i]);
		}

	kobject_put(info_kobj);
	kobject_put(root_kobj);


	//(void)platform_module_cleanup();


}

module_init(cnodes_init);
module_exit(cnodes_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pawel Narwojsz <narwpaw@gmail.com>");
