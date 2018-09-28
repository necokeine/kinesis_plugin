#include "include/eosio/kinesis_plugin/kinesis_producer.hpp"

int main(int argc, char **argv)
{
	unsigned char buf[512] = "abcdefghijkl";
    eosio::kinesis_producer producer;
    producer.kinesis_init();

    producer.kinesis_sendmsg(1, buf, 12);

    producer.kinesis_destory();
}


        std::random_device rd;
        std::mt19937 mt_rand(rd());

        Aws::Client::ClientConfiguration clientConfig;
        // set your region
        clientConfig.region = Aws::Region::AP_NORTHEAST_1;
        Aws::Kinesis::KinesisClient kinesisClient(clientConfig);

        Aws::Vector<Aws::String> animals{"dog", "cat", "owl", "horse", "stoat", "snake"};
        Aws::Kinesis::Model::PutRecordsRequest putRecordsRequest;
        putRecordsRequest.SetStreamName(streamName);
        Aws::Vector<Aws::Kinesis::Model::PutRecordsRequestEntry> putRecordsRequestEntryList;

        // create 500 records
        std::cout << "Adding records to stream \"" << streamName << "\"" << std::endl;
        for (int i = 0; i < 500; i++)
        {
            Aws::Kinesis::Model::PutRecordsRequestEntry putRecordsRequestEntry;
            Aws::StringStream pk;
            pk << "pk-" << (i % 100);
            putRecordsRequestEntry.SetPartitionKey(pk.str());
            Aws::StringStream data;
            data << i << ", " << animals[mt_rand() % animals.size()] << ", " << mt_rand() << ", " << mt_rand() * (float).001;
            Aws::Utils::ByteBuffer bytes((unsigned char*)data.str().c_str(), data.str().length());
            putRecordsRequestEntry.SetData(bytes);
            putRecordsRequestEntryList.emplace_back(putRecordsRequestEntry);
        }
        putRecordsRequest.SetRecords(putRecordsRequestEntryList);
        Aws::Kinesis::Model::PutRecordsOutcome putRecordsResult = kinesisClient.PutRecords(putRecordsRequest);

        // if one or more records were not put, retry them
        while (putRecordsResult.GetResult().GetFailedRecordCount() > 0)
        {
            std::cout << "Some records failed, retrying" << std::endl;
            Aws::Vector<Aws::Kinesis::Model::PutRecordsRequestEntry> failedRecordsList;
            Aws::Vector<Aws::Kinesis::Model::PutRecordsResultEntry> putRecordsResultEntryList = putRecordsResult.GetResult().GetRecords();
            for (unsigned int i = 0; i < putRecordsResultEntryList.size(); i++)
            {
                Aws::Kinesis::Model::PutRecordsRequestEntry putRecordRequestEntry = putRecordsRequestEntryList[i];
                Aws::Kinesis::Model::PutRecordsResultEntry putRecordsResultEntry = putRecordsResultEntryList[i];
                if (putRecordsResultEntry.GetErrorCode().length() > 0)
                    failedRecordsList.emplace_back(putRecordRequestEntry);
            }
            putRecordsRequestEntryList = failedRecordsList;
            putRecordsRequest.SetRecords(putRecordsRequestEntryList);
            putRecordsResult = kinesisClient.PutRecords(putRecordsRequest);
        }
        // Describe shards
        Aws::Kinesis::Model::DescribeStreamRequest describeStreamRequest;
        describeStreamRequest.SetStreamName(streamName);
        Aws::Vector<Aws::Kinesis::Model::Shard> shards;
        Aws::String exclusiveStartShardId = "";
        do
        {
            Aws::Kinesis::Model::DescribeStreamOutcome describeStreamResult = kinesisClient.DescribeStream(describeStreamRequest);
            Aws::Vector<Aws::Kinesis::Model::Shard> shardsTemp = describeStreamResult.GetResult().GetStreamDescription().GetShards();
            shards.insert(shards.end(), shardsTemp.begin(), shardsTemp.end());
            std::cout << describeStreamResult.GetError().GetMessage();
            if (describeStreamResult.GetResult().GetStreamDescription().GetHasMoreShards() && shards.size() > 0)
            {
                exclusiveStartShardId = shards[shards.size() - 1].GetShardId();
                describeStreamRequest.SetExclusiveStartShardId(exclusiveStartShardId);
            }
            else
                exclusiveStartShardId = "";
        } while (exclusiveStartShardId.length() != 0);

        if (shards.size() > 0)
        {
            std::cout << "Shards found:" << std::endl;
            for (auto shard : shards)
            {
                std::cout << shard.GetShardId() << std::endl;
            }

            Aws::Kinesis::Model::GetShardIteratorRequest getShardIteratorRequest;
            getShardIteratorRequest.SetStreamName(streamName);
            // use the first shard found
            getShardIteratorRequest.SetShardId(shards[0].GetShardId());
            getShardIteratorRequest.SetShardIteratorType(Aws::Kinesis::Model::ShardIteratorType::TRIM_HORIZON);

            Aws::Kinesis::Model::GetShardIteratorOutcome getShardIteratorResult = kinesisClient.GetShardIterator(getShardIteratorRequest);
            Aws::String shardIterator = getShardIteratorResult.GetResult().GetShardIterator();

            Aws::Kinesis::Model::GetRecordsRequest getRecordsRequest;
            getRecordsRequest.SetShardIterator(shardIterator);
            getRecordsRequest.SetLimit(25);

            // pull down 100 records
            std::cout << "Retrieving 100 records" << std::endl;
            for (int i = 0; i < 4; i++)
            {
                Aws::Kinesis::Model::GetRecordsOutcome getRecordsResult = kinesisClient.GetRecords(getRecordsRequest);
                for (auto r : getRecordsResult.GetResult().GetRecords())
                {
                    Aws::String s((char*)r.GetData().GetUnderlyingData());
                    std::cout << s.substr(0, r.GetData().GetLength()) << std::endl;
                }
                shardIterator = getRecordsResult.GetResult().GetNextShardIterator();
            }
        }
    }