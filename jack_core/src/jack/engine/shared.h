// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025


#ifndef JACK_SHARED_H
#define JACK_SHARED_H

#include <string>  // for string
namespace flatbuffers { class FlatBufferBuilder; }

namespace aos { namespace jack {
namespace shared {

struct EngineModel;

/*! ***********************************************************************************************
 * @class   SharedMemoryModel
 *
 * A shared memory model for sharing internal information with development tools
 * ************************************************************************************************/
class SharedMemoryModel
{
public:
    /* *********************************************************************************************
     * Public constants
     * ********************************************************************************************/
    const static std::string SM_FLAG;

public:
    /* *********************************************************************************************
     * Public Constructors and Destructors
     * ********************************************************************************************/
    
    /// Constructs a SharedMemoryModel 
    /// @param server Is this the server? (true by default)
    SharedMemoryModel   (bool server = false);

    /// Deconstruct the shared memory model
    ~SharedMemoryModel  ();

public:
    /* *********************************************************************************************
     *  Public Accessors
     * ********************************************************************************************/
    
    // @return The status (0: detached, 1: attached:, -1 error)
    int                 status          () const  {
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
        return m_status;
#else
        return 0;
#endif
    }

    /* *********************************************************************************************
     *  Public Functions
     * ********************************************************************************************/
    
    /// Attach to the shared memory
    /// @return true if we succesfully attached
    bool                attach          ();

    /// Detach from the shared memory
    void                detach          ();

    /// Write to the shared memory model
    /// @param fbb The buffer to write
    void                writeEngine     (flatbuffers::FlatBufferBuilder& fbb) const;

    /// Read from the shared memory model
    /// @return EngineModel
    const EngineModel   *readEngine     ();

    /// Update the memory
    unsigned long       updateMemory    ();
private:
    /* *********************************************************************************************
     *  Private Attributes
     * ********************************************************************************************/
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    int             m_key;
    char            *m_localdata;
    int             m_sem;
    key_t           m_semkey;
    bool            m_server;
    char            *m_shareddata;

    // The size in bytes of the shared memory block
    unsigned long   m_shmsiz;
    int             m_status;
#endif // JACK_SHARED_MEMORY_DEBUGGING
};

}}} // namespace aos::jack::shared
#endif // JACK_SHARED_H
