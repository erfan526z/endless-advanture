#pragma once

#include <thread>
#include <string>
#include "Chunk.h"
#include "Queue.h"

/*
This is the thread for processing load/generate/remesh/save/delete request for chunks.
More info:
This thread uses three queues (load/generate, save/delete, remesh) for each operations.
Once a Chunk is enqueued the process will automaticaly execute, for each task a flag will be 'true' until the job is finished.
The flag means that the Chunk is still in queue and you may not do other operations at the same time.
See Also:
chunk_thread::initManagerThread, chunk_thread::enqueueLoadRequest, chunk_thread::enqueueSaveRequest, chunk_thread::enqueueMeshRequest, chunk_thread::saveAndKill
*/
int chunkManagerThread();

namespace chunk_thread
{
	/* Before running the thread, you may initialize the world name and the world save directory address using this function.
	The information is used when saveing/loading the chunk data.*/
	void initManagerThread(const char* world_name, const char* save_addr, int seeds[16]);
	
	/* You can enqueue a Chunk which you want to load or generate the data.
	After a call the 'load_requested' flag on Chunk will be set until the data is in place.
	While the flag is set, not request more requests and do not change variables on the Chunk object.*/
	void enqueueLoadRequest(Chunk* chunk);

	/* You can enqueue a Chunk which you want to save and delete the data.
	After a call the 'unload_requested' flag on Chunk will be set until the chunk is wiped as a new one.
	While the flag is set, it means the chunk is about to get deleted, Don't touch it and let it go :)*/
	void enqueueSaveRequest(Chunk* chunk);

	/* You can enqueue a Chunk which you want to update the chunk mesh.
	After a call the 'mesh_update_requested' flag on Chunk will be set. THE FLAG WILL REMAIN SET UNTIL YOU UPDATE VRAM DATA USING updateVRAM().
	When the mesh buffer is generated, new_mesh_ready flag will be set and you should send the buffer to the GPU in the thread which you are using for OpenGL.
	While the flag is set, not request more requests and do not change variables on the Chunk object.*/
	void enqueueMeshRequest(Chunk* chunk);

	/* Cancels any pending request, saves any unsaved data from the list. AUTOMATICALY CALLED ON destroy() METHOD ON CHUNK MANAGER*/
	void saveAndKill(Chunk* chunklist, int len);

	/* Checks if the thread is ready for requests. */
	bool isInitialized();

}