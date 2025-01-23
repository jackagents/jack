#include <jack/engine/shared.h>

// only works in linux at the moment
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/sem.h>
#endif

namespace flatbuffers { class FlatBufferBuilder; }
namespace aos { namespace jack { namespace shared {
struct EngineModel;

/* *********************************************************************************************
 * Public constants
 * ********************************************************************************************/
const std::string SharedMemoryModel::SM_FLAG = "USE_SHARED_MEMORY";

SharedMemoryModel::SharedMemoryModel(bool server)
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    : m_key         (-1)
    , m_localdata   (new char[65535])
    , m_server      (server)
    , m_shareddata  ((char*)-1)
    , m_shmsiz      (65535)
    , m_status      (0)
#endif
{
    // maybe unused
    (void)server;

    /* ********************************************************************************************
     * Create a new shared memory model for the engine
     * Server flag tells if the caller is the responsible for the memory,
     * that is if it can create it and eventually demanding its destruction
     * *******************************************************************************************/
     attach(); // Automatically attempt to attach to shared memory
}

SharedMemoryModel::~SharedMemoryModel()
{
    // Explicity disconnect when being destroyed
    detach();
}

bool SharedMemoryModel::attach()
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /* ********************************************************************************************
     * Initialize and connect to the shared memory
     * *******************************************************************************************/
    // ftok to generate unique key
    key_t key = ftok("/tmp",65);

    // verify key is properly generated
    if (key < 0)
    {
        JACK_ERROR("Shared memory key fail: " << std::strerror(errno));
        m_status = -1;
        return false;
    }

    // shmget returns an identifier in shmid
    m_key = shmget(key, m_shmsiz, m_server ? IPC_CREAT|0666 : 0666);
    if ((long) m_key < 0)
    {
        JACK_ERROR("Shared memory id fail: " << std::strerror(errno));
        m_status = -1;
        return false;
    }

    // shmat to attach to shared memory
    m_shareddata = (char*) shmat(m_key,(void*)0,0);
    if ((long) m_shareddata < 0)
    {
        JACK_ERROR("Shared memory data fail: " << std::strerror(errno));
        m_status = -1;
        return false;
    }

    // Get the key for the jack shared memory semaphore
    m_semkey = ftok("/tmp", 1);
    if (m_semkey < 0)
    {
        JACK_ERROR("Semaphore key fail: " << std::strerror(errno));
        m_status = -1;
        return false;
    }

    // Create the semaphore if server, otherwise retrieve it
    m_sem = semget(m_semkey, 1, m_server ? IPC_CREAT|0666 : 0666);
    if (m_sem < 0)
    {
        JACK_ERROR("Semaphore id fail: " << std::strerror(errno));
        m_status = -1;
        return false;
    }

    // Verify this is the server
    if (m_server)
    {
        // Initialize the value of the semaphore to 1
        int r = semctl(m_sem, 0, SETVAL, 1);
        if (r == -1)
        {
            JACK_ERROR("Semaphore init fail: " << std::strerror(errno));
            m_status = -1;
            return false;
        }
    }

    m_status = 1;
    return true;
#else
    return false;
#endif
}

void SharedMemoryModel::detach()
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /* ********************************************************************************************
     * Disconnect the shared memory pointer and request memory destroy if owner
     * *******************************************************************************************/
    // Reset status back to 0
    m_status = 0;
    // Detach from shared memory
    if ((long) m_shareddata > -1)
    {
        shmdt(m_shareddata);
    }

    // Remove the shared memory if key was valid and this is a server
    if ((long) m_key > -1 && m_server) {
        // mark the shared memory for destroy if owner
        if (shmctl(m_key,IPC_RMID,NULL) < 0)
        {
            // memory will be removed when all the processes detach
            JACK_ERROR("Cannot mark shared memory for removal" << std::strerror(errno));
        }
    }

    // Rempve tje semaphore if key was valid and this is a server
    if (m_semkey > -1 && m_server) {
        // mark the semaphore for removal
        int rc = semctl( m_semkey, 1, IPC_RMID );
        if (rc==-1)
        {
            JACK_ERROR("Cannot mark semaphore for removal" << std::strerror(errno));
        }
    }
#endif
}

void SharedMemoryModel::writeEngine(flatbuffers::FlatBufferBuilder& fbb) const
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /* ********************************************************************************************
     * Write the given buffer into the shared memory
     * *******************************************************************************************/
    // Sanity check
    if ((long) m_shareddata < 0) return;

    // Get buffer data
    uint8_t* buf = fbb.GetBufferPointer();
    unsigned long bufsiz = fbb.GetSize();
    // Verify buffer size is not too big
    if (bufsiz > m_shmsiz)
    {
        // Ok if this happens the other side will receive a bad buffer! expect failures!
        JACK_WARNING("Engine buffer doesn't fit in the available memory block: " << bufsiz << " > " << m_shmsiz);
    }

    // Semaphore decrement operation setup, used to acquire
    struct sembuf semdecrement;
    semdecrement.sem_op     = -1;           // Decrement by one
    semdecrement.sem_flg    = 0;            // Wait
    semdecrement.sem_num    = 0;            // Operate on the first and only semaphore

    // Semaphore increment operation setup, used to release
    struct sembuf semincrement;
    semincrement.sem_op     = +1;           // Increment by one
    semincrement.sem_flg    = IPC_NOWAIT;   // Don't wait to release
    semincrement.sem_num    = 0;             // Operate on the first semaphore

    // Decrement semaphore (blocking call)
    semop(m_sem, &semdecrement, 1);

    // Copy over to the shared memory (max bufsiz)
    std::memcpy(m_shareddata, buf, std::min(bufsiz, m_shmsiz));

    // Increment semaphore (non-blocking call)
    semop(m_sem, &semincrement, 1);
#endif
    // maybe unused
    (void)fbb;
}

unsigned long SharedMemoryModel::updateMemory()
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /* ********************************************************************************************
     * Used to regularly update the local data from the shared memory
     * Return the buffer declared size, zero if invalid
     * *******************************************************************************************/
    // Sanity checks
    if (m_key < 0 || (long)m_shareddata < 0)
    {
        JACK_ERROR("Cannot update memory: shared segment detached");
        return 0;
    }

    // Semaphore decrement operation setup, used to acquire
    struct sembuf semdecrement;
    semdecrement.sem_op     = -1;           // Decrement by one
    semdecrement.sem_flg    = 0;            // Wait
    semdecrement.sem_num    = 0;            // Operate on the first and only semaphore

    // Semaphore increment operation setup, used to release
    struct sembuf semincrement;
    semincrement.sem_op     = +1;           // Increment by one
    semincrement.sem_flg    = IPC_NOWAIT;   // Don't wait to release
    semincrement.sem_num    = 0;             // Operate on the first semaphore

    // Decrement semaphore (blocking call)
    semop(m_sem, &semdecrement, 1);

    // Retrieve shared memory size
    struct shmid_ds buf;
    int ctl = shmctl(m_key, IPC_STAT, &buf);
    if (ctl < 0)
    {
        JACK_ERROR("Cannot retrieve engine model: shared memory info unavailable: " << std::strerror(errno));
        return 0;
    }
    // Use the actual shared memory segment size (which my differ from this very version)
    unsigned long size = buf.shm_segsz;
    // Copy the valid data into the local area
    std::min(size, m_shmsiz);
    std::memcpy(m_localdata, m_shareddata, std::min(size, m_shmsiz));

    // Increment semaphore (non-blocking call)
    semop(m_sem, &semincrement, 1);

    return size;
#else
    return 0;
#endif
}

const EngineModel* SharedMemoryModel::readEngine()
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /* ********************************************************************************************
     * Attempts to read a JACK engine proxy from the shared memory
     * Returns nullptr if no engine can be read
     * *******************************************************************************************/
    // Ensure local memory is up to date
    unsigned long size = updateMemory();
    if(!size)
    {
        return nullptr;
    }
    // Verify shared memory content is valid
    const unsigned char *data = (const unsigned char*)m_localdata;
    flatbuffers::Verifier verifier = flatbuffers::Verifier(data, size);
    bool ok = VerifyEngineModelBuffer(verifier);
    if (!ok)
    {
        JACK_ERROR("Cannot retrieve engine model: shared memory content verification failed");
        //\todo: find a nice way to retrieve a valid engine model with inconsistent data
        return nullptr;
    } else
    {
        // If data is valid, generate the engine model
        return GetEngineModel(data);
    }
#else
    return nullptr;
#endif // JACK_SHARED_MEMORY_DEBUGGING
}

}}} // namespace aos::jack::shared
