#include <netinet/in.h>

#include "utility.h"

std::string macToString(uint8_t *mac) {
    std::stringstream buff;
    buff << std::hex;

    for(int i = 0; i < 6; i++) {
        buff << static_cast<int>(mac[i]);
        if(i != 5)
            buff << ":";
    }
    return buff.str();
}

void stringToMac(const std::string& macString, uint8_t *out) {
    unsigned int bytes[6];
    if (std::sscanf(macString.c_str(),
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    &bytes[0], &bytes[1], &bytes[2],
                    &bytes[3], &bytes[4], &bytes[5]) != 6)
    {
        throw std::runtime_error("invalid MAC address: " + macString);
    }
    for(int i = 0; i < 6; i++) {
        out[i] = bytes[i];
    }
}

std::string ipv4ToString(uint32_t ipv4) {
    std::stringstream stream;
    stream << (ipv4 & 0xFFu) << "." << ((ipv4 >> 8u) & 0xFFu) << "." << ((ipv4 >> 16u) & 0xFFu) << "." << ((ipv4 >> 24u) & 0xFFu);
    return stream.str();
}

uint32_t stringToIpv4(const std::string& ipv4_str) {
    return 0;
}

uint16_t checksum(uint16_t *ptr, uint32_t len, uint32_t sum)
{
    // Credit: 	http://www.csee.usf.edu/~kchriste/tools/checksum.c
    //			https://github.com/chobits/tapip
    uint16_t			odd_byte;

    /*
     * Our algorithm is simple, using a 32-bit accumulator (sum),
     * we add sequential 16-bit words to it, and at the end, fold back
     * all the carry bits from the top 16 bits into the lower 16 bits.
     */
    while (len > 1)  {
        sum += *ptr++;
        len -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (len == 1) {
        odd_byte = 0;		/* make sure top half is zero */
        *((uint8_t *) &odd_byte) = *(uint8_t *)ptr;   /* one byte only */
        sum += odd_byte;
    }

    /*
     * Add back carry outs from top 16 bits to low 16 bits.
     */

    sum  = (sum >> 16u) + (sum & 0xffffu);	/* add high-16 to low-16 */
    sum += (sum >> 16u);			/* add carry */
    return (uint16_t)~sum;
}

uint16_t tcp_checksum(void *tcp_segment, uint16_t tcp_segment_len, uint32_t source_ip, uint32_t dest_ip) {
    // We need to include the pseudo-header in the checksum.
    uint32_t sum = htons(IPPROTO_TCP)
                   + htons(tcp_segment_len)
                   + source_ip
                   + dest_ip;

    return checksum((uint16_t *)tcp_segment, (uint32_t) (tcp_segment_len), sum);
}