/*
 * LkrCrateSlotDecoder.h
 *
 *  Created on: Aug 9, 2017
 *      Author: root
 */

#ifndef STRUCTS_LKRCRATESLOTDECODER_H_
#define STRUCTS_LKRCRATESLOTDECODER_H_



struct lkr_crate_slot_decoder {
    public:
        lkr_crate_slot_decoder(uint16_t source_sub_id) : buffer(source_sub_id){}

        uint_fast8_t getCrate() {
           return (uint_fast8_t) ((buffer & 0x7E0) >> 5); // Masking 11111100000
        }
        uint_fast8_t getSlot() {
           return (uint_fast8_t) (buffer & 0x1F); // Masking 00000011111
        }
    private:
        uint_fast16_t buffer;
}__attribute__ ((__packed__));


#endif /* STRUCTS_LKRCRATESLOTDECODER_H_ */
