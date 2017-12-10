#pragma once

void continuos_copy(void* dst, long& offset, const void* src, const size_t size);

void dump(void* indata, size_t size, const string& header = "");
