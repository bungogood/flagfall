#![allow(dead_code)]
/// [TODO] i32, i64, String
/// Also the QWORD part haven't been tested yet.
extern crate serialport; 

use std::io; 
use serialport::SerialPort; 

/// Tries to read a raw QWORD from the given `port`.
/// 
/// This function gives no concern to endianness. 
pub fn read_qword_raw(port: &mut dyn SerialPort) -> Result<u64, io::Error> {
    let mut buf = [0 as u8; 8]; 
    port.read_exact(&mut buf)?;
    let qword_ptr: *const u64 = (&buf as *const u8).cast(); 
    unsafe {
        return Ok(*qword_ptr); 
    }
}

/// Tries to read a raw QWORD from the given `port`, 
/// then converts it to the opposite endian.
/// 
/// Useful for, say, reading x86-based numeric values on an ARM machine. 
pub fn read_qword_flipped_endian(port: &mut dyn SerialPort) -> Result<u64, io::Error> {
    let mut buf = [0 as u8; 8]; 
    port.read_exact(&mut buf)?; 
    let qword: u64 = // [TODO] Prob. no need for all that -- reversal + casting should work fine
        ((buf[0] as u64) << 56 & 0xff00_0000_0000_0000) |
        ((buf[1] as u64) << 48 &   0xff_0000_0000_0000) |
        ((buf[2] as u64) << 40 &      0xff00_0000_0000) |
        ((buf[3] as u64) << 32 &        0xff_0000_0000) |
        ((buf[4] as u64) << 24 &           0xff00_0000) |
        ((buf[5] as u64) << 16 &             0xff_0000) |
        ((buf[6] as u64) <<  8 &                0xff00) |
        ((buf[7] as u64)       &                  0xff); 
    return Ok(qword); 
}

/// Tries to write a raw QWORD to the given `port`.
/// 
/// This function gives no concern to endianness. 
pub fn write_qword_raw(port: &mut dyn SerialPort, ref val: u64) -> Result<(), io::Error> {
    let buf_ptr: *const [u8; 8] = (val as *const u64).cast(); 
    unsafe {
        return port.write_all(&*buf_ptr); 
    }
}

/// Tries to write a QWORD with flipped endian to the given `port`.
/// 
/// Useful for, say, writing x86-based numerics to ARM machines.
pub fn write_qword_flipped_endian(port: &mut dyn SerialPort, ref mut val: u64) -> Result<(), io::Error> {
    let buf_ptr: *mut [u8; 8] = (val as *mut u64).cast(); 
    unsafe {
        let buf: &mut [u8; 8] = &mut *buf_ptr; 
        buf.reverse();
        return port.write_all(buf); 
    }
}

/// Tries to read a raw QWORD from the given `port` and converts it into `i64`. 
/// 
/// This function gives no concern to endianness. 
pub fn read_i64_raw(port: &mut dyn SerialPort) -> Result<i64, io::Error> {
    Ok(read_qword_raw(port)? as i64)
}

/// Tries to read a raw DWORD from the given `port`. 
/// 
/// This function gives no concern to endianness. 
pub fn read_dword_raw(port: &mut dyn SerialPort) -> Result<u32, io::Error> {
    let mut buf = [0 as u8; 4]; 
    port.read_exact(&mut buf)?;
    let dword_ptr: *const u32 = (&buf as *const u8).cast();
    unsafe {
        return Ok(*dword_ptr); 
    }
}

/// Tries to read a raw DWORD from the given `port`, 
/// then converts it to the opposite endian. 
/// 
/// Useful for, say, reading x86-based numeric values on an ARM machine. 
pub fn read_dword_flipped_endian(port: &mut dyn SerialPort) -> Result<u32, io::Error> {
    let mut buf = [0 as u8; 4]; 
    port.read_exact(&mut buf)?; 
    let dword: u32 = 
        ((buf[0] as u32) << 24 & 0xff00_0000) |
        ((buf[1] as u32) << 16 &   0xff_0000) | 
        ((buf[2] as u32) <<  8 &      0xff00) |
        ((buf[3] as u32)       &        0xff); 
    return Ok(dword); 
}

/// Tries to write a raw DWORD to the given `port`. 
/// 
/// This function gives no concern to endianness. 
pub fn write_dword_raw(port: &mut dyn SerialPort, ref val: u32) -> Result<(), io::Error> {
    let buf_ptr: *const [u8; 4] = (val as *const u32).cast(); 
    unsafe {
        return port.write_all(&*buf_ptr); 
    }
}

/// Tries to write a DWORD with flipped endian to the given `port`. 
/// 
/// Useful for, say, writing x86-based numerics to ARM machines. 
pub fn write_dword_flipped_endian(port: &mut dyn SerialPort, ref mut val: u32) -> Result<(), io::Error> {
    let buf_ptr: *mut [u8; 4] = (val as *mut u32).cast(); 
    unsafe {
        let buf: &mut [u8; 4] = &mut *buf_ptr;
        buf.reverse(); 
        return port.write_all(buf); 
    }
}

/// Tries to read a raw DWORD from the given `port` and converts it into `i64`. 
/// 
/// This function gives no concern to endianness. 
pub fn read_i32_raw(port: &mut dyn SerialPort) -> Result<i32, io::Error> {
    Ok(read_dword_raw(port)? as i32)
}