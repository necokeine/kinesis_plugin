#ifndef KINESIS_PRODUCER_HPP
#define KINESIS_PRODUCER_HPP

#include <aws/core/Aws.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/kinesis/KinesisClient.h>
#include <aws/kinesis/model/PutRecordRequest.h>
#include <aws/kinesis/model/PutRecordRequest.h>
#include <aws/core/utils/Outcome.h>

#include <string>

namespace eosio {

class kinesis_producer {
 public:
  kinesis_producer() {}

  int kinesis_init(const std::string& stream_name, const std::string& region_name) {
    // m_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info; // Turn on log.
    Aws::InitAPI(m_options);

    Aws::Client::ClientConfiguration clientConfig;
    // set your region
    clientConfig.region = Aws::Utils::HashingUtils::HashString(region_name);
    m_client = new Aws::Kinesis::KinesisClient(clientConfig);
    m_putRecordsRequest.SetStreamName(stream_name);
    m_counter = 0;
    return 0;
  }

  int kinesis_initialize_msg() {
    Aws::Kinesis::Model::PutRecordsRequest putRecordsRequest;
    putRecordsRequest.SetStreamName(m_streamName);
    m_putRecordsRequestEntryList.clear();
  }

  int kinesis_add_msg(const string& msg) {
    Aws::Kinesis::Model::PutRecordsRequestEntry putRecordsRequestEntry;
    Aws::StringStream pk;
    pk << "pk-" << (m_counter++ % 100);
    putRecordsRequestEntry.SetPartitionKey(pk.str());
    Aws::StringStream data;
    data << msg;
    Aws::Utils::ByteBuffer bytes((unsigned char*)data.str().c_str(), data.str().length());
    putRecordsRequestEntry.SetData(bytes);
    putRecordsRequestEntryList.emplace_back(putRecordsRequestEntry);
  }

  int kinesis_commit(int trxtype, unsigned char *msgstr, size_t length) {

    
    Aws::Kinesis::Model::PutRecordsOutcome putRecordsResult = m_client.PutRecords(m_putRecordsRequest);


    putRecordsRequest.SetStreamName(m_streamName);
    putRecordsRequestEntryList.clear();

    // trxtype => to different stream.
    Aws::Kinesis::Model::PutRecordRequest request;
    request.SetStreamName(m_streamName);
    Aws::Utils::ByteBuffer data(msgstr, length);
    request.SetData(data);

    auto result = m_client->PutRecord(request);
    if (!result.IsSuccess()) {
      return 1;
    }
    return 0;
  }

  int kinesis_destory() {
    delete m_client;
    Aws::ShutdownAPI(m_options);
    return 0;
  }
  
 private:
  Aws::SDKOptions m_options;
  Aws::Kinesis::KinesisClient *m_client;
  Aws::String m_streamName;

  Aws::Kinesis::Model::PutRecordsRequest m_putRecordsRequest;
  Aws::Vector<Aws::Kinesis::Model::PutRecordsRequestEntry> m_putRecordsRequestEntryList;

  uint64_t m_counter;
};

}  // namespace eosio

#endif
