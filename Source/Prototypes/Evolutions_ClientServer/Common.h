#pragma pack(push,1)
struct VectorStruct {
	double x, y, z;
};
#pragma pack(pop)

enum PacketInfoType {
	none = 0, vector, integer, str
};