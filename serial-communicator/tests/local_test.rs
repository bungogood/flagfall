#![cfg(unix)]

extern crate serial_communicator; 

use std::time::Duration;
use serialport::{SerialPort, TTYPort, Result}; 
use serial_communicator::*; 

const TEST_QWORD: u64                = 0xcafe_beef_dead_acab; 
const TEST_QWORD_FLIPPED_ENDIAN: u64 = 0xabac_adde_efbe_feca; 
const TEST_DWORD: u32                = 0xcafe_beef; 
const TEST_DWORD_FLIPPED_ENDIAN: u32 = 0xefbe_feca; 
const TEST_I64: i64                  = i64::MIN; 
const TEST_I32: i32                  = i32::MIN; 

fn _set_up() -> Result<(TTYPort, TTYPort)> {
    TTYPort::pair()
}

fn _write_raw_read_raw_u32(tx: &mut TTYPort, rx: &mut TTYPort) {
    write_dword_raw(tx, TEST_DWORD)
        .expect("[write_raw_read_raw_u32] Cannot write to `tx`"); 
    if let Ok(dword) = read_dword_raw(rx) {
        assert_eq!(
            dword, 
            TEST_DWORD, 
            "[ERROR] `read_dword_raw` received incorrect DWORD at `rx` read"
        ); 
    } else {
        panic!("[write_raw_read_raw_u32] Cannot read at `rx`"); 
    }
}

fn _write_flipped_read_raw_u32(tx: &mut TTYPort, rx: &mut TTYPort) {
    write_dword_flipped_endian(tx, TEST_DWORD)
        .expect("[write_flipped_read_raw_u32] Cannot write to `tx`"); 
    if let Ok(dword) = read_dword_raw(rx) {
        assert_eq!(
            dword, 
            TEST_DWORD_FLIPPED_ENDIAN, 
            "[ERROR] `read_dword_raw` received incorrect DWORD at `rx` read"
        ); 
    } else {
        panic!("[write_flipped_read_raw_u32] Cannot read at `rx`"); 
    }
}

fn _write_raw_read_flipped_u32(tx: &mut TTYPort, rx: &mut TTYPort) {
    write_dword_raw(tx, TEST_DWORD)
        .expect("[write_raw_read_flipped_u32] Cannot write to `tx`"); 
    if let Ok(dword) = read_dword_flipped_endian(rx) {
        assert_eq!(
            dword, 
            TEST_DWORD_FLIPPED_ENDIAN, 
            "[ERROR] `read_dword_flipped_endian` received incorrect DWORD at `rx` read"
        ); 
    } else {
        panic!("[write_raw_read_flipped_u32] Cannot read at `rx`"); 
    }
}

fn _write_raw_read_raw_u64(tx: &mut TTYPort, rx: &mut TTYPort) {
    todo!()
}

#[test]
fn test_ttyport_pairs() {
    let (mut tx, mut rx) = _set_up()
        .expect("[local_test::set_up] Cannot create pseudo TTY ports");
    
    tx.set_timeout(Duration::from_millis(10))
        .expect("[local_test::test_ttyport_pairs] Cannot set timeout on `tx`");
    rx.set_timeout(Duration::from_millis(10))
        .expect("[local_test::test_ttyport_pairs] Cannot set timeout on `rx`"); 

    // DWORD
    _write_raw_read_raw_u32(&mut tx, &mut rx); 
    _write_flipped_read_raw_u32(&mut tx, &mut rx); 
    _write_raw_read_flipped_u32(&mut tx, &mut rx); 

    // QWORD
    write_qword_raw(&mut tx, TEST_QWORD)
        .expect("[serial_communicator::write_qword_raw] Cannot write to `tx`"); 
    if let Ok(qword) = read_qword_raw(&mut rx) {
        assert_eq!(qword, TEST_QWORD, "[ERROR] `read_qword_raw` receives incorrect QWORD at `rx` read"); 
    } else {
        panic!("[serial_communicator::read_qword_raw] Cannot read at `rx`"); 
    }
}

