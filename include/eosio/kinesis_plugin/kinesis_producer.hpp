#ifndef KINESIS_PRODUCER_HPP
#define KINESIS_PRODUCER_HPP

#include <aws/core/Aws.h>
#include <aws/kinesis/KinesisClient.h>
#include <aws/kinesis/model/PutRecordRequest.h>
#include <aws/kinesis/model/PutRecordRequest.h>
#include <aws/core/utils/Outcome.h>

namespace eosio {
const char *kSTREAM_NAME = "EOS_Asia_Kinesis";

class kinesis_producer {
 public:
  kinesis_producer() {}

  int kinesis_init() {
    // m_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info; // Turn on log.
    Aws::InitAPI(m_options);

    Aws::Client::ClientConfiguration clientConfig;
    // set your region
    clientConfig.region = Aws::Region::AP_NORTHEAST_1;
    m_client = new Aws::Kinesis::KinesisClient(clientConfig);
    return 0;
  }

  int kinesis_sendmsg(int trxtype, unsigned char *msgstr, size_t length) {
    // trxtype => to different stream.
    Aws::Kinesis::Model::PutRecordRequest request;
    request.SetStreamName(kSTREAM_NAME);
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
};

}  // namespace eosio

#endif
