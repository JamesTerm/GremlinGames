#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "networktables2/type/NetworkTableEntryTypeManager.h"
#include "networktables2/type/DefaultEntryTypes.h"
#include "networktables2/connection/DataIOStream.h"
#include "ByteArrayIOStream.h"
#include "TesterTemplate.h"
#include "networktables2/util/IOException.h"

TEST(EntryType, testIdToType) {
		EXPECT_EQ(&DefaultEntryTypes::BOOLEAN, (new NetworkTableEntryTypeManager())->GetType(0));
		EXPECT_EQ(&DefaultEntryTypes::DOUBLE, (new NetworkTableEntryTypeManager())->GetType(1));
		EXPECT_EQ(&DefaultEntryTypes::STRING, (new NetworkTableEntryTypeManager())->GetType(2));
		EXPECT_EQ(NULL, (new NetworkTableEntryTypeManager())->GetType(3));
		EXPECT_EQ(NULL, (new NetworkTableEntryTypeManager())->GetType(4));
		EXPECT_EQ(NULL, (new NetworkTableEntryTypeManager())->GetType(5));
		EXPECT_EQ(NULL, (new NetworkTableEntryTypeManager())->GetType(127));
		EXPECT_EQ(NULL, (new NetworkTableEntryTypeManager())->GetType(-1));
	}
TEST(EntryType, testTypeIds) {
		EXPECT_EQ(DefaultEntryTypes::BOOLEAN.id, 0);
		EXPECT_EQ(DefaultEntryTypes::DOUBLE.id, 1);
		EXPECT_EQ(DefaultEntryTypes::STRING.id, 2);
	}
TEST(EntryType, testTypeNames) {
		EXPECT_STREQ(DefaultEntryTypes::BOOLEAN.name, "Boolean");
		EXPECT_STREQ(DefaultEntryTypes::DOUBLE.name, "Double");
		EXPECT_STREQ(DefaultEntryTypes::STRING.name, "String");
	}

	

TEST(EntryType, testBooleanRead) {
		
			NEW_BAIOS(1,0)
			EXPECT_FALSE(DefaultEntryTypes::BOOLEAN.readValue(*(new DataIOStream(input))).b);
			delete input;
		
			UPDATE_BAIOS(1,1)
			EXPECT_TRUE(DefaultEntryTypes::BOOLEAN.readValue(*(new DataIOStream(input))).b);
			delete input;
			

			UPDATE_BAIOS(0)
			ASSERT_THROW(DefaultEntryTypes::BOOLEAN.readValue(*(new DataIOStream(input))), IOException);
			delete input;
	}
TEST(EntryType, testBooleanWrite) {
			
			ByteArrayIOStream *output = new ByteArrayIOStream();
			EntryValue ev;
			ev.b = false;
			DefaultEntryTypes::BOOLEAN.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expA[1] = {0};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expA, 1));
		
		output = new ByteArrayIOStream();
			ev.b = true;
			DefaultEntryTypes::BOOLEAN.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expb[1] = {1};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expb, 1));
			}
			/* TODO: fix this throw
		try {
			ByteArrayOutputStream output = new ByteArrayOutputStream();
			DefaultEntryTypes.BOOLEAN.sendValue(new Object(), new DataOutputStream(output));
			fail();
		} catch (IOException e) {}
	}
	
	
	*/
		
TEST(EntryType, testDoubleRead) {
			NEW_BAIOS(8, 64, 41, 0, 0, 0, 0, 0, 0)
			EXPECT_EQ(12.5, DefaultEntryTypes::DOUBLE.readValue(*(new DataIOStream(input))).f);
			delete input;
		
			UPDATE_BAIOS(8, 64, 100, 0, 0, 0, 0, 0, 0)
			EXPECT_EQ(160, DefaultEntryTypes::DOUBLE.readValue(*(new DataIOStream(input))).f);
			delete input;

			UPDATE_BAIOS(0)
			ASSERT_THROW(DefaultEntryTypes::DOUBLE.readValue(*(new DataIOStream(input))), IOException);
			delete input;
		
	}
	
	TEST(EntryType, testDoubleWrite) {
			
			ByteArrayIOStream *output = new ByteArrayIOStream();
			EntryValue ev;
			ev.f = 12.5;
			DefaultEntryTypes::DOUBLE.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expA[] = {64, 41, 0, 0, 0, 0, 0, 0};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expA, 8));
		
		output = new ByteArrayIOStream();
			ev.f = 160;
			DefaultEntryTypes::DOUBLE.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expb[] = {64, 100, 0, 0, 0, 0, 0, 0};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expb, 8));
			}
	/*
	@Te TODO: cache
		try {
			ByteArrayOutputStream output = new ByteArrayOutputStream();
			DefaultEntryTypes.DOUBLE.sendValue(new Object(), new DataOutputStream(output));
			fail();
		} catch (IOException e) {}
	}

	
	
	
	
	

	
*/
TEST(EntryType, testStringRead) {
			NEW_BAIOS(10, 0, 8, 77, 121, 32, 69, 110, 116, 114, 121)
			EXPECT_STREQ("My Entry", ((std::string*)DefaultEntryTypes::STRING.readValue(*(new DataIOStream(input))).ptr)->c_str());
			delete input;
		
			UPDATE_BAIOS(7, 0, 5, 86, 97, 76, 117, 69)
			EXPECT_STREQ("VaLuE", ((std::string*)DefaultEntryTypes::STRING.readValue(*(new DataIOStream(input))).ptr)->c_str());
			delete input;
		
			UPDATE_BAIOS(0)
			ASSERT_THROW(DefaultEntryTypes::STRING.readValue(*(new DataIOStream(input))), IOException);
			delete input;
		
	}

		TEST(EntryType, testStringWrite) {
			
			ByteArrayIOStream *output = new ByteArrayIOStream();
			EntryValue ev;
			ev.ptr = new std::string("My Entry");
			DefaultEntryTypes::STRING.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expA[] = {0, 8, 77, 121, 32, 69, 110, 116, 114, 121};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expA, 10));
		
		output = new ByteArrayIOStream();
			ev.ptr = new std::string("VaLuE");
			DefaultEntryTypes::STRING.sendValue(ev, *(new DataIOStream(output)));
			uint8_t expb[] = {0, 5, 86, 97, 76, 117, 69};
			EXPECT_THAT(output->toByteArray(), IsArrayEqualTo(expb, 7));
			}
	/*TODO: throw
		try {
			ByteArrayOutputStream output = new ByteArrayOutputStream();
			DefaultEntryTypes.STRING.sendValue(new Object(), new DataOutputStream(output));
			fail();
		} catch (IOException e) {}
	}

}*/
