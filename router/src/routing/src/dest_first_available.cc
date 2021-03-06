/*
  Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "dest_first_available.h"
#include "mysql/harness/logging/logging.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

IMPORT_LOG_FUNCTIONS()

int DestFirstAvailable::get_server_socket(
    std::chrono::milliseconds connect_timeout, int *error,
    mysql_harness::TCPAddress *address) noexcept {
  if (destinations_.empty()) {
    return -1;
  }

  for (size_t i = 0; i < destinations_.size(); ++i) {
    // We start at the currently available server
    auto addr = destinations_.at(current_pos_);
    log_debug("Trying server %s (index %lu)", addr.str().c_str(),
              static_cast<long unsigned>(i));  // 32bit Linux requires cast
    auto sock = get_mysql_socket(addr, connect_timeout);
    if (sock >= 0) {
      if (address) *address = addr;
      return sock;
    } else {
      if (++current_pos_ >= destinations_.size()) current_pos_ = 0;
    }
  }

#ifndef _WIN32
  *error = errno;
#else
  *error = WSAGetLastError();
#endif
  return -1;
}
