// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_PROTOCOL_HELPERS_H
#define JACK_PROTOCOL_HELPERS_H

#include <jack/corelib.h>
#include <jack/message.h>
#include <vector>

namespace aos::jack
{
 // I've disabled all this for now - not sure it's needed

//Field jackFieldToProtocolField(const jack::Field& field);

/// Convert a JACK message to a list of JACK protocol any (protocol variant
/// data type).
/// @param[in] src The Message to convert
/// @param[in] dest The array of fields to convert to
//std::vector<Field> jackMessageToProtocolFieldVector(const Message* src);

//bool protocolFieldVectorToJACKMessage(const std::vector<jack::Field>& src, Message* msg);

} /// namespace aos::jack
#endif /// JACK_PROTOCOL_HELPERS_H
