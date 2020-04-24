#pragma once

#include <libusb.h>
#include <string>
#include <unordered_set>
#include <vector>
#include "UsbException.h"

namespace libusb
{
class device;

class context
{
public:
    context() : _context( nullptr )
    {
        if ( libusb_init( &_context ) < 0 )
        {
            throw UsbException( "Couldn't init usb" );
        }
        libusb_set_debug( _context, LIBUSB_LOG_LEVEL_INFO );
    }
    ~context() noexcept
    {
        libusb_exit( _context );
    }
    device open_device_with_vid_pid( uint16_t vendor_id, uint16_t product_id );

    operator libusb_context*() const { return _context; }
    
private:
    libusb_context* _context;
};

class device_descriptor : protected libusb_device_descriptor
{
    friend class device;
public:
    uint8_t length() const { return bLength; }
	uint8_t descriptorType() const { return bDescriptorType; }
	uint16_t usbSpec() const { return bcdUSB; }
	uint8_t  deviceClass() const { return bDeviceClass; }
	uint8_t  deviceSubClass() const { return bDeviceSubClass; }
	uint8_t  deviceProtocol() const { return bDeviceProtocol; }
	uint8_t  maxPacketSize() const { return bMaxPacketSize0; }
	uint16_t vendorId() const { return idVendor; }
	uint16_t productId() const { return idProduct; }
	uint16_t deviceRelease() const { return bcdDevice; }
	std::string manufacturer() const
    {
        std::string str( 255U, '\0' );
        auto len = libusb_get_string_descriptor_ascii(
            _dev, iManufacturer, reinterpret_cast< uint8_t* >( &str[0] ), str.size() );
        str.resize( len > 0 ? len : 0 );
        return str;
    }
	std::string product() const
    {
        std::string str( 255U, '\0' );
        auto len = libusb_get_string_descriptor_ascii(
            _dev, iProduct, reinterpret_cast< uint8_t* >( &str[0] ), str.size() );
        str.resize( len > 0 ? len : 0 );
        return str;
    }
	std::string serialNumber() const
    {
        std::string str( 255U, '\0' );
        auto len = libusb_get_string_descriptor_ascii(
            _dev, iSerialNumber, reinterpret_cast< uint8_t* >( &str[0] ), str.size() );
        str.resize( len > 0 ? len : 0 );
        return str;
    }
	uint8_t numConfigurations() const { return bNumConfigurations; }

private:
    libusb_device_handle* _dev = nullptr;
};

class interface
{
private:
    libusb_interface _interface;
};

class config_descriptor
{
    friend class device;
public:
    config_descriptor( config_descriptor&& rval )
    {
        *this = std::move( rval );
    }
    ~config_descriptor()
    {
        if ( _config_descriptor != nullptr )
        {
            libusb_free_config_descriptor( _config_descriptor );
        }
    }

	uint8_t descriptorType() const { return _config_descriptor->bDescriptorType; }
	uint8_t configurationValue() const { return _config_descriptor->bConfigurationValue; }
	std::string configuration() const
    {
        std::string str( 255U, '\0' );
        auto len = libusb_get_string_descriptor_ascii(
            _dev, _config_descriptor->iConfiguration, reinterpret_cast< uint8_t* >( &str[0] ), str.size() );
        str.resize( len > 0 ? len : 0 );
        return str;
    }
	uint8_t attributes() const { return _config_descriptor->bmAttributes; }
	uint8_t maxPower() const { return _config_descriptor->MaxPower; }
    const std::vector<libusb_interface>& interfaces() const { return _interfaces; }
    const std::vector<uint8_t>& extra() const { return _extra; }

    void operator=( config_descriptor&& rval )
    {
        _config_descriptor = std::move( rval._config_descriptor );
        rval._config_descriptor = nullptr;
        _interfaces = std::move( rval._interfaces );
        _extra = std::move( rval._extra );
    }
private:
    libusb_device_handle* _dev;
    libusb_config_descriptor* _config_descriptor;
    std::vector<libusb_interface> _interfaces;
    std::vector<uint8_t> _extra;

    config_descriptor( libusb_device_handle* dev, libusb_device* device ) : _dev( dev ), _config_descriptor( nullptr )
    {
        auto r = libusb_get_active_config_descriptor( device, &_config_descriptor );
        if ( r < 0 )
        {
            throw UsbException( libusb_strerror( static_cast< libusb_error >( r ) ) );
        }

        _interfaces.reserve( _config_descriptor->bNumInterfaces );
        for ( uint8_t i = 0; i < _config_descriptor->bNumInterfaces; ++i )
        {
            _interfaces.push_back( _config_descriptor->interface[i] );
        }
        _extra.assign( _config_descriptor->extra, _config_descriptor->extra + _config_descriptor->extra_length );
    }
};

class device
{
    friend class context;
public:
    device() : _dev( nullptr ), _device( nullptr )
    {}
    device(device&& rval)
    {
        _dev = std::move( rval._dev );
        rval._dev = nullptr;
        _device = std::move( rval._device );
        rval._device = nullptr;
        _device_descriptor = std::move( rval._device_descriptor );
        _claimedInterfaces = std::move( rval._claimedInterfaces );
    }
    ~device() noexcept
    {
        for ( auto i : _claimedInterfaces )
        {
            libusb_release_interface( _dev, i );
        }
        libusb_close(_dev);
    }

    const device_descriptor& get_device_descriptor()
    {
        if ( _device == nullptr )
        {
            _device = libusb_get_device( _dev );
            libusb_get_device_descriptor( _device, &_device_descriptor );
            _device_descriptor._dev = _dev;
        }
        return _device_descriptor;
    }
    void detach_kernel_driver( int interface_number )
    {
        if ( libusb_kernel_driver_active( _dev, interface_number ) != 1 )
        {
            return;
        }
        
        auto r = libusb_detach_kernel_driver( _dev, interface_number );
        if ( r < 0 )
        {
            throw UsbException( libusb_strerror( static_cast< libusb_error >( r ) ) );
        }
    }
    void claim_interface( int interface_number )
    {
        auto r = libusb_claim_interface( _dev, interface_number );
        if ( r < 0 )
        {
            throw UsbException( libusb_strerror( static_cast< libusb_error >( r ) ) );
        }
        _claimedInterfaces.insert ( interface_number );
    }
    void set_interface_alt_setting( int interface_number, int alternate_setting )
    {
        auto r = libusb_set_interface_alt_setting( _dev, interface_number, alternate_setting );
        if ( r < 0 )
        {
            throw UsbException( libusb_strerror( static_cast< libusb_error >( r ) ) );
        }
    }
    config_descriptor get_active_config_descriptor()
    {
        if ( _device == nullptr )
        {
            _device = libusb_get_device( _dev );
            libusb_get_device_descriptor( _device, &_device_descriptor );
            _device_descriptor._dev = _dev;
        }

        config_descriptor config( _dev, _device );
        return config;
    }
    operator libusb_device_handle*()
    {
        return _dev;
    }

private:
    device( libusb_device_handle* dev ) : _dev( dev ), _device( nullptr )
    {}
    device( const device& ) = delete;

    libusb_device_handle* _dev;
    libusb_device* _device;
    device_descriptor _device_descriptor;
    std::unordered_set<int> _claimedInterfaces;
};

inline device context::open_device_with_vid_pid( uint16_t vendor_id, uint16_t product_id )
{
    auto dev_handle = libusb_open_device_with_vid_pid( _context, vendor_id, product_id );
    if ( dev_handle == nullptr )
    {
        throw UsbException( "Couldn't open device" );
    }
    return device( dev_handle );
}
}
