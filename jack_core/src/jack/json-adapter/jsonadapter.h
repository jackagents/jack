#ifndef JACK_JSON_ADAPTER_H
#define JACK_JSON_ADAPTER_H

/// Protocol
#include <jack/event-protocol/busadapter.h>

/// Third Party
#include <fstream>
#include <string_view>

namespace aos
{
class JSONAdapter : public jack::BusAdapter
{
    /**************************************************************************
     * Functions
     **************************************************************************/
public:
    /// Converts all JACK protocol events to JSON and streams to file
    JSONAdapter(std::string_view filePath);

    /// Open the file stream backing this adapter for writing into
    bool connect() override final;

    /// Close the file stream backing this adapter containing the written events
    void disconnect() override final;

    /// Translate the protocol event to JSON for writing to the opened file.
    ///
    /// If the adapter has not opened the file this function will no-op and 
    /// print a warning on the first invocation.
    ///
    bool sendEvent(const jack::protocol::Event* event) override final;

    /**************************************************************************
     * Fields
     **************************************************************************/
public:
    /// Set the indent level of the JSON. When set to 0, no indent is applied
    /// and the adapter will serialised each line to exactly 1 event.
    uint8_t       m_indent          = 0;

    /// Emit the events into a global JSON array when true ensuring that the
    /// final output file constitutes a valid JSON file. Otherwise emit each
    /// event as a standalone object. Avoiding the global array is useful for
    /// consumers to stream the output of the JSON file line-by-line when
    /// combined with `m_indent == 0`.
    ///
    /// This value must only be set prior to `connect()` or after `disconnect()`
    /// has been called.
    bool          m_emitGlobalArray = false;

    uint64_t      m_eventsWritten   = 0;
private:
    std::string   m_filePath;
    std::ofstream m_file;
};
} // namespace aos
#endif /// JACK_JSON_ADAPTER_H
