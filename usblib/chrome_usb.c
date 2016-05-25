/*
* Chrome API USB support
*
* Copyright (c) 2016 Nikita Batov <nikitabatov@gmail.com>
*
* This library is covered by the LGPL, read LICENSE for details.
*/

#include <stdlib.h> /* getenv, etc */
#include <unistd.h>
#include <string.h>

#include "chrome_usb.h"
#include "usbi.h"
#include <emscripten.h>

#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif


void usb_os_find_busses_st(void);
void usb_os_find_devices_st(int i);
void usb_os_open_st(void);
void usb_os_close_st(void);
void usb_claim_interface_st(void);
void usb_release_interface_st(void);
void usb_bulk_write_st(void);
void usb_bulk_read_st(void);
void usb_interrupt_write_st(void);
void usb_interrupt_read_st(void);


void usb_os_find_busses_cb(int dev_cnt);
void usb_os_find_devices_cb(int vid, int pid, int did);
void usb_os_open_cb(int handle_id);
void usb_os_close_cb(int handle_id);
void usb_claim_interface_cb(void);
void usb_release_interface_cb(void);
void usb_bulk_write_cb(void);
void usb_bulk_read_cb(void);
void usb_interrupt_write_cb(void);
void usb_interrupt_read_cb(void);


void usb_cb_wait(void);

void ready_to_cont(void);

_Bool ready_to_continue = 0;

void usb_cb_wait(void)
{
    int pr = 0;
    while(!ready_to_continue)
    {
        emscripten_sleep_with_yield(200);
    }
    //if (ready_to_continue)
    // EM_ASM({console.log("Callback wait released");});
}


usb_dev_handle *mdev;
int minterface;
int mep;
char *mbytes;
int msize;
int mtimeout;
int g_handle = 0x00;

int read_cnt = 0;

void usb_os_open_st()
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    
    EM_ASM_(
    {
        var device = {};
        
        device["device"] = $0;
        device["manufacturerName"] = "";
        device["productId"] = $2;
        device["productName"] = "";
        device["serialNumber"] = "";
        device["vendorId"] = $1;
        
        
        var usb_os_open_cb = Module.cwrap
        ('usb_os_open_cb',
        'number',
        ['number']);
        
        chrome.usb.openDevice(device, function(handle)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            
            usb_os_open_cb(handle['handle']);
            console.log(handle['handle']);
        });
        
        
    },did,vid,pid);
}

int usb_os_open(usb_dev_handle *dev)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_os_open start")});
    #endif
    
    ready_to_continue = 0;
    mdev = dev;
    usb_os_open_st();
    
    usb_cb_wait();
    
    dev->fd = g_handle;
    
    return 0;
}

void usb_os_close_st()
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    EM_ASM_(
    {
        var handle = {};
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        
        chrome.usb.closeDevice(handle, function()
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            ready_to_cont();
        });
        
        
    },handle_id,vid,pid);
}

int usb_os_close(usb_dev_handle *dev)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_os_close");});
    #endif
    
    ready_to_continue = 0;
    mdev = dev;
    usb_os_close_st();
    
    usb_cb_wait();
    
    dev->fd = 0x00;
    
    return 0;
}

int usb_set_configuration(usb_dev_handle *dev, int configuration)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_set_configuration");});
    #endif
    
    return 0;
}

void usb_claim_interface_st()
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    EM_ASM_(
    {
        var handle = {};
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        var ino = $3;
        
        console.log("Try to claim: handle = "+ handle["handle"]+", productId = " + handle["productId"] + ", vendorId=" + handle["vendorId"] + ", interface = " + ino);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        
        chrome.usb.claimInterface(handle, ino ,function()
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
                chrome.usb.closeDevice(handle, function(){console.log("Closed!")});
            }
            else
            console.log("Claim success!");
            ready_to_cont();
        });
        
        
    },handle_id,vid,pid,minterface);
}


int usb_claim_interface(usb_dev_handle *dev, int interface)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_claim_interface");});
    #endif
    
    ready_to_continue = 0;
    mdev = dev;
    minterface = interface;
    usb_claim_interface_st();
    
    usb_cb_wait();
    
    return 0;
}


void usb_release_interface_st()
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    EM_ASM_(
    {
        var handle = {};
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        var ino = $3;
        
        console.log("Try to release: handle = "+ handle["handle"]+", productId = " + handle["productId"] + ", vendorId=" + handle["vendorId"] + ", interface = " + ino);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        
        chrome.usb.releaseInterface(handle, ino ,function()
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            console.log("Claim success!");
            ready_to_cont();
        });
        
        
    },handle_id,vid,pid,minterface);
}


int usb_release_interface(usb_dev_handle *dev, int interface)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_release_interface");});
    
    #endif
    ready_to_continue = 0;
    mdev = dev;
    minterface = interface;
    usb_claim_interface_st();
    
    usb_cb_wait();
    
    return 0;
}

int usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_set_altinterface");});
    #endif
    
    return 0;
}


int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
int value, int index, char *bytes, int size, int timeout)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_control_msg");});
    #endif
    
    return 0;
}

static int usb_urb_transfer(usb_dev_handle *dev, int ep, int urbtype,
char *bytes, int size, int timeout)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_urb_transfer");});
    #endif
    
    return 0;
}

void usb_bulk_write_st(void)
{
    
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    /*for (int i = 0; i<8;i++)
    EM_ASM_({console.log("data " + $0 + " = " + $1)},i,mbytes[i]);*/
    
    EM_ASM_(
    {
        var handle = {};
        var transfer_info = {};
        var data = new Uint8Array($5);
        
        for (var i = 0; i<$5; i++)
        data[i] = Module.getValue($4+i,"i8");
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        
        transfer_info['direction'] = 'out';
        transfer_info['endpoint'] = $3;
        transfer_info['data'] = data.buffer;
        transfer_info['timeout'] = $6;
        
        //console.log("endpoint of write = " +  transfer_info['endpoint']);
        // console.log("timeout of write = " +  transfer_info['timeout']);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        chrome.usb.bulkTransfer(handle,transfer_info, function(result)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            if (result['resultCode'] != 0)
            console.log("Write ERROR!");
            
            
            //console.log(result);
            ready_to_cont();
        });
        
        
        
    },handle_id,vid,pid,mep,mbytes,msize,mtimeout);
}

int usb_bulk_write(usb_dev_handle *dev, int ep, char *bytes, int size,
int timeout)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_bulk_write");});
    #endif
    
    ready_to_continue = 0;
    mdev = dev;
    mep = ep;
    mbytes = bytes;
    msize = size;
    mtimeout = timeout;
    usb_bulk_write_st();
    usb_cb_wait();
    
    //EM_ASM({console.log("after write");});
    //exit(777);
    return 0;
}


void usb_bulk_read_st(void)
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    /*for (int i = 0; i<8;i++)
    EM_ASM_({console.log("data " + $0 + " = " + $1)},i,mbytes[i]);*/
    
    EM_ASM_(
    {
        var handle = {};
        var transfer_info = {};
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        
        transfer_info['direction'] = 'in';
        transfer_info['endpoint'] = $3;
        transfer_info['length'] = $5;
        transfer_info['timeout'] = $6;
        
        //console.log("endpoint of read = " +  transfer_info['endpoint']);
        //console.log("length of read = " +  transfer_info['length']);
        //console.log("timeout of read = " +  transfer_info['timeout']);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        chrome.usb.bulkTransfer(handle,transfer_info, function(result)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            if (result['resultCode'] != 0)
            console.log("Read ERROR!");
            
            //console.log(result);
            //console.log(result['data']);
            //console.log(result['data'].buffer);
            var dv = new DataView(result.data);
            
            for (var i = 0; i < $5; ++i)
            {
                //console.log(dv.getUint8(i));
                Module.setValue($4+i, dv.getUint8(i), "i8");
            }
            
            
            ready_to_cont();
        });
        
        
        
    },handle_id,vid,pid,mep,mbytes,msize,mtimeout);
}

int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
int timeout)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_bulk_read");});
    
    #endif
    ready_to_continue = 0;
    mdev = dev;
    mep = ep;
    mbytes = bytes;
    msize = size;
    mtimeout = timeout;
    if (timeout != 100)
    {
        usb_bulk_read_st();
        usb_cb_wait();
    }
    
    return 0;
}


int usb_interrupt_write(usb_dev_handle *dev, int ep, char *bytes, int size,
int timeout)
{
    #ifdef DEBUG
    printf("%sn", "usb_interrupt_write()");
    #endif
    
    
    ready_to_continue = 0;
    mdev = dev;
    mep = ep;
    mbytes = bytes;
    msize = size;
    mtimeout = timeout;
    usb_interrupt_write_st();
    usb_cb_wait();
    
    return 0;
}

void usb_interrupt_write_st()
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    /*for (int i = 0; i<8;i++)
    EM_ASM_({console.log("data " + $0 + " = " + $1)},i,mbytes[i]);*/
    
    EM_ASM_(
    {
        var handle = {};
        var transfer_info = {};
        var data = new Uint8Array($5);
        
        for (var i = 0; i<$5; i++)
        data[i] = Module.getValue($4+i,"i8");
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        
        transfer_info['direction'] = 'out';
        transfer_info['endpoint'] = $3;
        transfer_info['data'] = data.buffer;
        transfer_info['timeout'] = $6;
        
        //console.log("endpoint of write = " +  transfer_info['endpoint']);
        // console.log("timeout of write = " +  transfer_info['timeout']);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        chrome.usb.interruptTransfer(handle,transfer_info, function(result)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            if (result['resultCode'] != 0)
            console.log("Write ERROR!");
            
            
            //console.log(result);
            ready_to_cont();
        });
        
        
        
    },handle_id,vid,pid,mep,mbytes,msize,mtimeout);
}

void usb_interrupt_read_st(void)
{
    int pid = mdev->device->descriptor.idProduct;
    int vid = mdev->device->descriptor.idVendor;
    int did = mdev->device->devnum;
    int handle_id =  mdev->fd;
    
    /*for (int i = 0; i<8;i++)
    EM_ASM_({console.log("data " + $0 + " = " + $1)},i,mbytes[i]);*/
    
    EM_ASM_(
    {
        var handle = {};
        var transfer_info = {};
        
        handle["handle"] = $0;
        handle["productId"] = $2;
        handle["vendorId"] = $1;
        
        transfer_info['direction'] = 'in';
        transfer_info['endpoint'] = $3;
        transfer_info['length'] = $5;
        transfer_info['timeout'] = $6;
        
        //console.log("endpoint of read = " +  transfer_info['endpoint']);
        //console.log("length of read = " +  transfer_info['length']);
        //console.log("timeout of read = " +  transfer_info['timeout']);
        
        var ready_to_cont = Module.cwrap
        ('ready_to_cont',
        'number',
        []);
        
        chrome.usb.interruptTransfer(handle,transfer_info, function(result)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            if (result['resultCode'] != 0)
            console.log("Read ERROR!");
            
            //console.log(result);
            //console.log(result['data']);
            //console.log(result['data'].buffer);
            var dv = new DataView(result.data);
            
            for (var i = 0; i < $5; ++i)
            {
                //console.log(dv.getUint8(i));
                Module.setValue($4+i, dv.getUint8(i), "i8");
            }
            
            
            ready_to_cont();
        });
        
        
        
    },handle_id,vid,pid,mep,mbytes,msize,mtimeout);
}


int usb_interrupt_read(usb_dev_handle *dev, int ep, char *bytes, int size,
int timeout)
{
    #ifdef DEBUG
    printf("%sn", "usb_interrupt_read()");
    #endif
    
    ready_to_continue = 0;
    mdev = dev;
    mep = ep;
    mbytes = bytes;
    msize = size;
    mtimeout = timeout;
    if (timeout != 100)
    {
        usb_interrupt_read_st();
        usb_cb_wait();
    }
    
    return 0;
}


struct usb_bus *fbus = NULL;
int devices_count = 0;

void  usb_os_find_busses_st()
{
    EM_ASM(
    {
        var usb_os_find_busses_cb = Module.cwrap
        ('usb_os_find_busses_cb',
        'number',
        ['number']);
        chrome.usb.getDevices({}, function(devices)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            usb_os_find_busses_cb(devices.length);
        });
    });
    
}

int usb_os_find_busses(struct usb_bus **busses)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_os_find_busses");});
    #endif
    
    ready_to_continue = 0;
    
    usb_os_find_busses_st();
    
    usb_cb_wait();
    
    *busses = fbus;
    
    return 0;
}

struct usb_device *fdev = NULL;

void usb_os_find_devices_st(int i)
{
    EM_ASM_(
    {
        var usb_os_find_devices_cb = Module.cwrap
        ('usb_os_find_devices_cb',
        'number',
        ['number','number','number']);
        
        chrome.usb.getDevices({}, function(devices)
        {
            if (chrome.runtime.lastError)
            {
                console.warn(chrome.runtime.lastError.message);
            }
            else
            usb_os_find_devices_cb(devices[$0].vendorId, devices[$0].productId, devices[$0].device);
        });
    },i);
}

int usb_os_find_devices(struct usb_bus *bus, struct usb_device **devices)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_os_find_devices");});
    #endif
    
    for (int i = 0; i < devices_count; i++)
    {
        ready_to_continue = 0;
        usb_os_find_devices_st(i);
        usb_cb_wait();
    }
    
    *devices = fdev;
    return 0;
}

int usb_os_determine_children(struct usb_bus *bus)
{
    #ifdef DEBUG
    printf("%sn", "usb_os_determine_children()");
    #endif
    
    return 0;
}

static int check_usb_vfs(const char *dirname)
{
    #ifdef DEBUG
    printf("%sn", "check_usb_vfs()");
    #endif
    
    return 0;
}

void usb_os_init(void)
{
    #ifdef DEBUG
    EM_ASM({console.log("usb_os_init");});
    #endif
    
}

int usb_resetep(usb_dev_handle *dev, unsigned int ep)
{
    #ifdef DEBUG
    printf("%sn", "usb_resetep()");
    #endif
    return 0;
}

int usb_clear_halt(usb_dev_handle *dev, unsigned int ep)
{
    #ifdef DEBUG
    printf("%sn", "usb_clear_halt()");
    #endif
    
    return 0;
}

int usb_reset(usb_dev_handle *dev)
{
    #ifdef DEBUG
    printf("%sn", "usb_reset()");
    #endif
    
    return 0;
}

int usb_get_driver_np(usb_dev_handle *dev, int interface, char *name,
unsigned int namelen)
{
    #ifdef DEBUG
    printf("%sn", "usb_get_driver_np()");
    #endif
    
    return 0;
}

int usb_detach_kernel_driver_np(usb_dev_handle *dev, int interface)
{
    #ifdef DEBUG
    printf("%sn", "usb_detach_kernel_driver_np()");
    #endif
    
    return 0;
}

void usb_os_find_busses_cb(int dev_cnt)
{
    if (dev_cnt)
    {
        devices_count = dev_cnt;
        struct usb_bus *bus;
        bus = malloc(sizeof(struct usb_bus));
        memset((void *)bus, 0, sizeof(*bus));
        LIST_ADD(fbus, bus);
    }
    ready_to_cont();
    
}

void usb_os_find_devices_cb(int vid, int pid, int did)
{
    struct usb_device *dev;
    dev = malloc(sizeof(struct usb_device));
    memset((void *)dev, 0, sizeof(*dev));
    
    dev->descriptor.idVendor  = vid;
    dev->descriptor.idProduct = pid;
    dev->devnum = did;
    
    
    struct usb_interface_descriptor intr_descriptor;
    struct usb_interface intr;
    struct usb_config_descriptor config;
    
    intr_descriptor.bInterfaceClass = 2;
    
    intr.altsetting = &intr_descriptor;
    
    config.interface = &intr;
    config.bNumInterfaces = 2;
    
    dev->config = &config;
    
    LIST_ADD(fdev, dev);
    
    ready_to_cont();
    
    
}

void usb_os_open_cb(int handle_id)
{
    g_handle = handle_id;
    ready_to_cont();
    
}

void ready_to_cont()
{
    ready_to_continue = 1;
}