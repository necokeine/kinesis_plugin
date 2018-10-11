#ifndef KINESIS_PRODUCER_HPP
#define KINESIS_PRODUCER_HPP

#include <aws/core/Aws.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/kinesis/KinesisClient.h>
#include <aws/kinesis/model/PutRecordsResult.h>
#include <aws/kinesis/model/PutRecordsRequest.h>
#include <aws/kinesis/model/PutRecordsRequestEntry.h>
#include <aws/core/utils/Outcome.h>

#include <string>

using namespace std;

namespace eosio {

class kinesis_producer {
 public:
  kinesis_producer() {}

  int kinesis_init(const std::string& stream_name, const std::string& region_name) {
    // m_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info; // Turn on log.
    Aws::InitAPI(m_options);

    Aws::Client::ClientConfiguration clientConfig;
    std::cout << "Region Name: " << region_name << "; Stream Name: " << stream_name;
    // set your region
    //clientConfig.region = Aws::Utils::HashingUtils::HashString(region_name.c_str());
    // We can't use HashString here directly right now, but don't know the suspect.
    clientConfig.region = Aws::Region::AP_NORTHEAST_1;
    m_client = new Aws::Kinesis::KinesisClient(clientConfig);
    m_putRecordsRequest.SetStreamName(stream_name.c_str());
    m_putRecordsRequestEntryList.clear();
    m_counter = 0;
    return 0;
  }

  int kinesis_sendmsg(const std::string& msg) {
    //cout << msg << endl;
    Aws::Kinesis::Model::PutRecordsRequestEntry putRecordsRequestEntry;
    Aws::StringStream pk;
    pk << "pk-" << (m_counter++ % 100);
    putRecordsRequestEntry.SetPartitionKey(pk.str());
    Aws::StringStream data;
    data << msg;
    //data << "1, hehe, test, 111";
    Aws::Utils::ByteBuffer bytes((unsigned char*)data.str().c_str(), data.str().length());
    putRecordsRequestEntry.SetData(bytes);
    m_putRecordsRequestEntryList.emplace_back(putRecordsRequestEntry);

    // cout << "Current Size:" << m_putRecordsRequestEntryList.size() << endl;

    if (m_putRecordsRequestEntryList.size() > 1000) {
      kinesis_commit();
    }
    return 0;
  }

  void kinesis_commit() {
    m_putRecordsRequest.SetRecords(m_putRecordsRequestEntryList);
    Aws::Kinesis::Model::PutRecordsOutcome putRecordsResult = m_client->PutRecords(m_putRecordsRequest);
    int retry_counter = 0;
    
    // if one or more records were not put, retry them
    while (putRecordsResult.GetResult().GetFailedRecordCount() > 0) {
      std::cout << "Some records failed, retrying" << std::endl;
      Aws::Vector<Aws::Kinesis::Model::PutRecordsRequestEntry> failedRecordsList;
      Aws::Vector<Aws::Kinesis::Model::PutRecordsResultEntry> putRecordsResultEntryList = putRecordsResult.GetResult().GetRecords();
      for (unsigned int i = 0; i < putRecordsResultEntryList.size(); i++) {
        Aws::Kinesis::Model::PutRecordsRequestEntry putRecordRequestEntry = m_putRecordsRequestEntryList[i];
        Aws::Kinesis::Model::PutRecordsResultEntry putRecordsResultEntry = putRecordsResultEntryList[i];
        if (putRecordsResultEntry.GetErrorCode().length() > 0) {
          cout << "Error Message: " << putRecordsResultEntry.GetErrorMessage().c_str() << endl;
          failedRecordsList.emplace_back(putRecordRequestEntry);
        }
      }
      m_putRecordsRequestEntryList = failedRecordsList;
      retry_counter++;
      if (retry_counter > 5) {
        //return;
      }
      m_putRecordsRequest.SetRecords(m_putRecordsRequestEntryList);
      putRecordsResult = m_client->PutRecords(m_putRecordsRequest);
    }

    m_putRecordsRequestEntryList.clear();
  }

  int kinesis_destory() {
    while (m_putRecordsRequestEntryList.size() > 0) {
      kinesis_commit();
    }
    delete m_client;
    Aws::ShutdownAPI(m_options);
    return 0;
  }
  
 private:
  Aws::SDKOptions m_options;
  Aws::Kinesis::KinesisClient *m_client;

  Aws::Kinesis::Model::PutRecordsRequest m_putRecordsRequest;
  Aws::Vector<Aws::Kinesis::Model::PutRecordsRequestEntry> m_putRecordsRequestEntryList;

  uint64_t m_counter;
};

}  // namespace eosio

#endif
