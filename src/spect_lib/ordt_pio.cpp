//   Ordt 221031.01 autogenerated file 
//   Input: /projects/tropic01/work/operoutka/ts-spect/reg_map/reg_map.rdl
//   Parms: /projects/tropic01/work/operoutka/ts-spect/scripts/ordt_param_file
//   Date: Wed Dec 07 13:40:17 CET 2022
//

#include "ordt_pio_common.hpp"
#include "ordt_pio.hpp"

// ------------------ ordt_addr_elem methods ------------------

ordt_addr_elem::ordt_addr_elem(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : m_startaddress(_m_startaddress),
    m_endaddress(_m_endaddress) {
}

bool  ordt_addr_elem::containsAddress(const uint64_t &addr) {
  return ((addr >= m_startaddress) && (addr <= m_endaddress));
}

bool  ordt_addr_elem::isBelowAddress(const uint64_t &addr) {
  return (addr > m_endaddress);
}

bool  ordt_addr_elem::isAboveAddress(const uint64_t &addr) {
  return (addr < m_startaddress);
}

bool  ordt_addr_elem::hasStartAddress(const uint64_t &addr) {
  return (addr == m_startaddress);
}

void  ordt_addr_elem::update_child_ptrs() {
}

// ------------------ ordt_regset methods ------------------

ordt_addr_elem*  ordt_regset::findAddrElem(const uint64_t &addr) {
  int lo = 0;
  int hi = m_children.size()-1;
  int mid = 0;
  while (lo <= hi) {
     mid = (lo + hi) / 2;
     if (m_children[mid]->containsAddress(addr)) {
        //outElem = m_children[mid];
        return m_children[mid];
     }
     else if (m_children[mid]->isAboveAddress(addr))
        hi = mid - 1;
     else
        lo = mid + 1;
  }
  return nullptr;
}

ordt_regset::ordt_regset(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_addr_elem(_m_startaddress, _m_endaddress) {
}

int  ordt_regset::write(const uint64_t &addr, const ordt_data &wdata) {
     if (this->containsAddress(addr)) {
        childElem = this->findAddrElem(addr);
        if (childElem != nullptr) { return childElem->write(addr, wdata); }
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write to invalid address " << addr << " in regset\n";
  #endif
     return 8;
}

int  ordt_regset::read(const uint64_t &addr, ordt_data &rdata) {
     if (this->containsAddress(addr)) {
        childElem = this->findAddrElem(addr);
        if (childElem != nullptr) { return childElem->read(addr, rdata); }
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read to invalid address " << addr << " in regset\n";
  #endif
     rdata.clear();
     return 8;
}

// ------------------ ordt_reg methods ------------------

ordt_reg::ordt_reg(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_addr_elem(_m_startaddress, _m_endaddress) {
}

ordt_reg::ordt_reg(const ordt_reg &_old)
  : ordt_addr_elem(_old),
    m_mutex() {
}

void  ordt_reg::write(const ordt_data &wdata) {
}

int  ordt_reg::write(const uint64_t &addr, const ordt_data &wdata) {
     return 0;
}

void  ordt_reg::read(ordt_data &rdata) {
}

int  ordt_reg::read(const uint64_t &addr, ordt_data &rdata) {
     return 0;
}

// ------------------ ordt_rg_r_block_id methods ------------------

ordt_rg_r_block_id::ordt_rg_r_block_id(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_reg(_m_startaddress, _m_endaddress),
    f_id_code(0, 16, 0x30, r_std, w_none),
    f_rev_code(16, 4, 0, r_std, w_none) {
}

int  ordt_rg_r_block_id::write(const uint64_t &addr, const ordt_data &wdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write of reg ordt_rg_r_block_id at addr="<< addr << ", data=" << wdata.to_string() << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->write(wdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write to invalid address " << addr << " in reg ordt_rg_r_block_id\n";
  #endif
     return 8;
}

void  ordt_rg_r_block_id::write(const ordt_data &wdata) {
  std::lock_guard<std::mutex> m_guard(m_mutex);
  f_id_code.write(wdata);
  f_rev_code.write(wdata);
}

int  ordt_rg_r_block_id::read(const uint64_t &addr, ordt_data &rdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read of reg ordt_rg_r_block_id at addr="<< addr << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->read(rdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read to invalid address " << addr << " in reg ordt_rg_r_block_id\n";
  #endif
     rdata.clear();
     return 8;
}

void  ordt_rg_r_block_id::read(ordt_data &rdata) {
  rdata.clear();
  for (uint64_t widx=0; widx<((m_endaddress - m_startaddress + 1)/4); widx++) rdata.push_back(0);
  f_id_code.read(rdata);
  f_rev_code.read(rdata);
}

// ------------------ ordt_rg_r_command methods ------------------

ordt_rg_r_command::ordt_rg_r_command(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_reg(_m_startaddress, _m_endaddress),
    f_start(0, 1, 0x0, r_none, w_1set),
    f_soft_reset(1, 1, 0x0, r_none, w_std) {
}

int  ordt_rg_r_command::write(const uint64_t &addr, const ordt_data &wdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write of reg ordt_rg_r_command at addr="<< addr << ", data=" << wdata.to_string() << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->write(wdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write to invalid address " << addr << " in reg ordt_rg_r_command\n";
  #endif
     return 8;
}

void  ordt_rg_r_command::write(const ordt_data &wdata) {
  std::lock_guard<std::mutex> m_guard(m_mutex);
  f_start.write(wdata);
  f_soft_reset.write(wdata);
}

int  ordt_rg_r_command::read(const uint64_t &addr, ordt_data &rdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read of reg ordt_rg_r_command at addr="<< addr << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->read(rdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read to invalid address " << addr << " in reg ordt_rg_r_command\n";
  #endif
     rdata.clear();
     return 8;
}

void  ordt_rg_r_command::read(ordt_data &rdata) {
  rdata.clear();
  for (uint64_t widx=0; widx<((m_endaddress - m_startaddress + 1)/4); widx++) rdata.push_back(0);
  f_start.read(rdata);
  f_soft_reset.read(rdata);
}

// ------------------ ordt_rg_r_status methods ------------------

ordt_rg_r_status::ordt_rg_r_status(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_reg(_m_startaddress, _m_endaddress),
    f_idle(0, 1, 0x1, r_std, w_none),
    f_done(1, 1, 0x0, r_std, w_1clr),
    f_err(2, 1, 0x0, r_std, w_1clr) {
}

int  ordt_rg_r_status::write(const uint64_t &addr, const ordt_data &wdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write of reg ordt_rg_r_status at addr="<< addr << ", data=" << wdata.to_string() << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->write(wdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write to invalid address " << addr << " in reg ordt_rg_r_status\n";
  #endif
     return 8;
}

void  ordt_rg_r_status::write(const ordt_data &wdata) {
  std::lock_guard<std::mutex> m_guard(m_mutex);
  f_idle.write(wdata);
  f_done.write(wdata);
  f_err.write(wdata);
}

int  ordt_rg_r_status::read(const uint64_t &addr, ordt_data &rdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read of reg ordt_rg_r_status at addr="<< addr << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->read(rdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read to invalid address " << addr << " in reg ordt_rg_r_status\n";
  #endif
     rdata.clear();
     return 8;
}

void  ordt_rg_r_status::read(ordt_data &rdata) {
  rdata.clear();
  for (uint64_t widx=0; widx<((m_endaddress - m_startaddress + 1)/4); widx++) rdata.push_back(0);
  f_idle.read(rdata);
  f_done.read(rdata);
  f_err.read(rdata);
}

// ------------------ ordt_rg_r_int_ena methods ------------------

ordt_rg_r_int_ena::ordt_rg_r_int_ena(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_reg(_m_startaddress, _m_endaddress),
    f_int_done_en(0, 1, 0x0, r_std, w_std),
    f_int_err_en(1, 1, 0x0, r_std, w_std) {
}

int  ordt_rg_r_int_ena::write(const uint64_t &addr, const ordt_data &wdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write of reg ordt_rg_r_int_ena at addr="<< addr << ", data=" << wdata.to_string() << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->write(wdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> write to invalid address " << addr << " in reg ordt_rg_r_int_ena\n";
  #endif
     return 8;
}

void  ordt_rg_r_int_ena::write(const ordt_data &wdata) {
  std::lock_guard<std::mutex> m_guard(m_mutex);
  f_int_done_en.write(wdata);
  f_int_err_en.write(wdata);
}

int  ordt_rg_r_int_ena::read(const uint64_t &addr, ordt_data &rdata) {
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read of reg ordt_rg_r_int_ena at addr="<< addr << "\n";
  #endif
     if (this->hasStartAddress(addr)) {
        this->read(rdata);
        return 0;
     }
  #ifdef ORDT_PIO_VERBOSE
     std::cout << "--> read to invalid address " << addr << " in reg ordt_rg_r_int_ena\n";
  #endif
     rdata.clear();
     return 8;
}

void  ordt_rg_r_int_ena::read(ordt_data &rdata) {
  rdata.clear();
  for (uint64_t widx=0; widx<((m_endaddress - m_startaddress + 1)/4); widx++) rdata.push_back(0);
  f_int_done_en.read(rdata);
  f_int_err_en.read(rdata);
}

// ------------------ ordt_root methods ------------------

ordt_root::ordt_root()
  : ordt_root(0x0, 0xf) {
}

ordt_root::ordt_root(uint64_t _m_startaddress, uint64_t _m_endaddress)
  : ordt_regset(_m_startaddress, _m_endaddress),
    r_block_id(_m_startaddress + 0x0, _m_startaddress + 0x3),
    r_command(_m_startaddress + 0x4, _m_startaddress + 0x7),
    r_status(_m_startaddress + 0x8, _m_startaddress + 0xb),
    r_int_ena(_m_startaddress + 0xc, _m_startaddress + 0xf) {
  m_children.push_back(&r_block_id);
  m_children.push_back(&r_command);
  m_children.push_back(&r_status);
  m_children.push_back(&r_int_ena);
}

void  ordt_root::update_child_ptrs() {
  m_children.clear();
  m_children.push_back(&r_block_id);
  m_children.push_back(&r_command);
  m_children.push_back(&r_status);
  m_children.push_back(&r_int_ena);
}

