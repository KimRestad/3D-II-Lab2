#include "Buffer.h"

Buffer::Buffer()
	: mDevice(0), mBuffer(0)
{
}

HRESULT Buffer::Initialize(ID3D10Device* device, BufferInformation initDescription)
{
	HRESULT result;
	mDevice = device;
	mDescription = initDescription;

	D3D10_BUFFER_DESC bufferDesc;

	switch(mDescription.type)								// Specify what kind of buffer is created
	{
		case VertexBuffer:
			bufferDesc.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
			break;
		case IndexBuffer:
			bufferDesc.BindFlags		= D3D10_BIND_INDEX_BUFFER;
			break;
		default:
			return E_FAIL;
	}

	switch(mDescription.usage)								// Set how to use the buffer
	{
		case Buffer_Default:
			bufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;		// Contents never change
			bufferDesc.CPUAccessFlags	= 0;							// No CPU access is allowed (USAGE_IMMUTABLE)
			break;
		case CPURead:
			bufferDesc.Usage			= D3D10_USAGE_DYNAMIC;			// Contents can change
			bufferDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_READ;		// CPU is allowed to read (USAGE_DYNAMIC)
			break;
		case CPUWrite:
			bufferDesc.Usage			= D3D10_USAGE_DYNAMIC;			// Contents can change
			bufferDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;		// CPU is allowed to write (USAGE_DYNAMIC)
			break;
		default:
			bufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;		// Contents never change
			bufferDesc.CPUAccessFlags	= 0;							// No CPU access is allowed (USAGE_IMMUTABLE)
			break;
	}

	int bufferSize = mDescription.elementSize * mDescription.numberOfElements;

	bufferDesc.ByteWidth		= bufferSize;					// The size of the buffer to create (in bytes)
	bufferDesc.MiscFlags		= 0;							// No other flags needed

	D3D10_SUBRESOURCE_DATA subData;
	subData.pSysMem				= mDescription.firstElementPointer;

	result = mDevice->CreateBuffer(&bufferDesc,					// Pointer to description of the buffer to create
									  &subData,					// Pointer to data to initialize buffer with
									  &mBuffer);				// Out: where to put the created buffer 

	return result;
}

Buffer::~Buffer()
{
	SafeRelease(mBuffer);

	mDevice = NULL;
	mBuffer = NULL;
}

void Buffer::MakeActive()
{
	UINT offset = 0;

	switch(mDescription.type)
	{
		case VertexBuffer:
			mDevice->IASetVertexBuffers(0, 1, &mBuffer, &mDescription.elementSize, &offset);
			break;
		case IndexBuffer:
			mDevice->IASetIndexBuffer(mBuffer, DXGI_FORMAT_R32_UINT, offset);
			break;
	}
}

void Buffer::Map()
{
	//void* data;

	switch (mDescription.usage)
	{
		case CPUWrite:
			/*mBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &data);
			memcpy(data, mDescription.firstElementPointer, mDescription.elementSize * mDescription.numberOfElements);
			mBuffer->Unmap();*/
			break;
		case CPURead:
			//mBuffer->Map(D3D10_MAP_READ, 0, &data);
			break;
	}
}

int Buffer::GetSize()
{
	return mDescription.numberOfElements;
}