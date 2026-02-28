#ifndef GEMV_REQUEST_HH
#define GEMV_REQUEST_HH

#include <cstdint>

namespace gem5
{
struct GemvRequest {
    uint8_t  req_type;      
    uint64_t weight_addr;   
    uint8_t  rank_num;      
    uint32_t k;             
    uint32_t n;             
    uint8_t input_prec;     
    uint8_t weight_prec;    
    uint8_t scale_prec;     
    bool is_valid;           

    GemvRequest() : 
        req_type(2), weight_addr(0), rank_num(0), k(0), n(0),
        input_prec(0), weight_prec(0), scale_prec(0),
        is_valid(false) {}
};
} // namespace gem5

#endif // GEMV_REQUEST_HH
