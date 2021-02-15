---
layout: post
title:  "Chrome EME (MediaSession) - 1"
author: "KuroNeko"
comments: true
tags: [analysis, drm]
---

라온화이트햇 핵심연구팀 원요한


# Chrome EME (MediaSession) - 1

이 문서는 Chrome에서 사용하는 MediaSession EME 코드 분석으로 내부 구조를 파악하기 위해 작성되었습니다.

## 1.1 EME (Encrypted Media Extensions)

EME란, 미디어 컨텐츠의 저작권 보호를 위해 사용되는 DRM 방식 중 하나입니다. 그리고 브라우저에서 사용할 수 있도록 디자인되었습니다.

![/assets/202102/202102won.png](/assets/202102/202102won.png)

위의 그림과 같이 License 서버에서 받은 License를 MediaKeySession을 통해 내부 Content Decryption Module에 전달하며 해당 구조로 인해서 브라우저마다 암/복호화 방식의 차이가 존재하면 안되기 때문에 어느정도 정해진 암/복호화 알고리즘을 사용할 것으로 보입니다. 이에 따라서, `CMAF(Common Media Application Format)` 이 등장하였습니다.

![/assets/202102/202102won1.png](/assets/202102/202102won1.png)

해당 포맷은 MPEG-DASH, HLS 방식의 프로토콜을 통해 미디어 컨텐츠들을 조각내 스트리밍 방식으로 전송합니다. 이 방식은 기존에 사용하던 코덱과 같이 하나의 컨텐츠 파일을 모두 다운로드되기 전까지 미디어 재생이 불가능한 단점을 극복하기 위해 고안되었습니다. 또한, 컨턴츠의 암호화를 통해 DRM과 같은 부가적인 서비스까지 적용가능합니다. DRM은 위의 그림과 같이 CMAF 포맷에서 표준 암호화 규격으로 `CENC(Common Encryption/AES-CTR)`와 `CBCS(AES-CBC)` 를 사용합니다. 각각의 복호화 방식에 대해서 [http://cs.chromium.org](http://cs.chromium.org) 에서 CDM의 소스코드를 분석해 어떤식으로 코드가 구성되어있는지 확인해보겠습니다.

### 1.1.1 CENC(Common Encryption)

```cpp
// ./src/media/cdm/cenc_decryptor.cc 
scoped_refptr<DecoderBuffer> DecryptCencBuffer(
    const DecoderBuffer& input,
    const crypto::SymmetricKey& key) {
  const char* sample = reinterpret_cast<const char*>(input.data());
  const size_t sample_size = input.data_size();
  DCHECK(sample_size) << "No data to decrypt.";

  const DecryptConfig* decrypt_config = input.decrypt_config();
  DCHECK(decrypt_config) << "No need to call Decrypt() on unencrypted buffer.";
  DCHECK_EQ(EncryptionScheme::kCenc, decrypt_config->encryption_scheme());

  const std::string& iv = decrypt_config->iv();
  DCHECK_EQ(iv.size(), static_cast<size_t>(DecryptConfig::kDecryptionKeySize));

  crypto::Encryptor encryptor;
  if (!encryptor.Init(&key, crypto::Encryptor::CTR, "")) {
    DVLOG(1) << "Could not initialize decryptor.";
    return nullptr;
  }

  if (!encryptor.SetCounter(iv)) {
    DVLOG(1) << "Could not set counter block.";
    return nullptr;
  }

  const std::vector<SubsampleEntry>& subsamples = decrypt_config->subsamples();
  if (subsamples.empty()) {
    // ...
  }

  size_t total_encrypted_size = 0;
  for (const auto& subsample : subsamples)
    total_encrypted_size += subsample.cypher_bytes;

  // No need to decrypt if there is no encrypted data.
  if (total_encrypted_size == 0) {
    auto output = DecoderBuffer::CopyFrom(input.data(), sample_size);
    CopyExtraSettings(input, output.get());
    return output;
  }

  std::unique_ptr<uint8_t[]> encrypted_bytes(new uint8_t[total_encrypted_size]);
  CopySubsamples(subsamples, kSrcContainsClearBytes,
                 reinterpret_cast<const uint8_t*>(sample),
                 encrypted_bytes.get());

  base::StringPiece encrypted_text(
      reinterpret_cast<const char*>(encrypted_bytes.get()),
      total_encrypted_size);
  std::string decrypted_text;
  if (!encryptor.Decrypt(encrypted_text, &decrypted_text)) {
    DVLOG(1) << "Could not decrypt data.";
    return nullptr;
  }
  DCHECK_EQ(decrypted_text.size(), encrypted_text.size());

  scoped_refptr<DecoderBuffer> output = DecoderBuffer::CopyFrom(
      reinterpret_cast<const uint8_t*>(sample), sample_size);
  CopySubsamples(subsamples, kDstContainsClearBytes,
                 reinterpret_cast<const uint8_t*>(decrypted_text.data()),
                 output->writable_data());
  CopyExtraSettings(input, output.get());
  return output;
}
```

### 1.1.2 CBCS(AES-CBC)

```cpp
// ./src/media/cdm/cbcs_decryptor.cc
scoped_refptr<DecoderBuffer> DecryptCbcsBuffer(
    const DecoderBuffer& input,
    const crypto::SymmetricKey& key) {
  size_t sample_size = input.data_size();
  DCHECK(sample_size) << "No data to decrypt.";

  const DecryptConfig* decrypt_config = input.decrypt_config();
  DCHECK(decrypt_config) << "No need to call Decrypt() on unencrypted buffer.";
  DCHECK_EQ(EncryptionScheme::kCbcs, decrypt_config->encryption_scheme());

  DCHECK(decrypt_config->HasPattern());
  const EncryptionPattern pattern =
      decrypt_config->encryption_pattern().value();

  auto buffer = base::MakeRefCounted<DecoderBuffer>(sample_size);
  uint8_t* output_data = buffer->writable_data();
  buffer->set_timestamp(input.timestamp());
  buffer->set_duration(input.duration());
  buffer->set_is_key_frame(input.is_key_frame());
  buffer->CopySideDataFrom(input.side_data(), input.side_data_size());

  const std::vector<SubsampleEntry>& subsamples = decrypt_config->subsamples();
  if (subsamples.empty()) {
    // ...
  }

  if (!VerifySubsamplesMatchSize(subsamples, sample_size)) {
    DVLOG(1) << "Subsample sizes do not equal input size";
    return nullptr;
  }

  const uint8_t* src = input.data();
  uint8_t* dest = output_data;
  for (const auto& subsample : subsamples) {
    if (subsample.clear_bytes) {
      DVLOG(4) << "Copying clear_bytes: " << subsample.clear_bytes;
      memcpy(dest, src, subsample.clear_bytes);
      src += subsample.clear_bytes;
      dest += subsample.clear_bytes;
    }

    if (subsample.cypher_bytes) {
      DVLOG(4) << "Processing cypher_bytes: " << subsample.cypher_bytes
               << ", pattern(" << pattern.crypt_byte_block() << ","
               << pattern.skip_byte_block() << ")";
      if (!DecryptWithPattern(
              key, base::as_bytes(base::make_span(decrypt_config->iv())),
              pattern, base::make_span(src, subsample.cypher_bytes), dest)) {
        return nullptr;
      }
      src += subsample.cypher_bytes;
      dest += subsample.cypher_bytes;
    }
  }

  return buffer;
}
```

```cpp
bool DecryptWithPattern(const crypto::SymmetricKey& key,
                        base::span<const uint8_t> iv,
                        const EncryptionPattern& pattern,
                        base::span<const uint8_t> input_data,
                        uint8_t* output_data) {
  // The AES_CBC decryption is reset for each subsample.
  AesCbcCrypto aes_cbc_crypto;
  if (!aes_cbc_crypto.Initialize(key, iv))
    return false;

  // |total_blocks| is the number of blocks in the buffer, ignoring any
  // partial block at the end. |remaining_bytes| is the number of bytes
  // in the partial block at the end of the buffer, if any.
  size_t total_blocks = input_data.size_bytes() / kAesBlockSizeInBytes;
  size_t remaining_bytes = input_data.size_bytes() % kAesBlockSizeInBytes;

  size_t crypt_byte_block =
      base::strict_cast<size_t>(pattern.crypt_byte_block());
  size_t skip_byte_block = base::strict_cast<size_t>(pattern.skip_byte_block());

  // |crypt_byte_block| and |skip_byte_block| come from 4 bit values, so fail
  // if these are too large.
  if (crypt_byte_block >= 16 || skip_byte_block >= 16)
    return false;

  if (crypt_byte_block == 0 && skip_byte_block == 0) {
    // From ISO/IEC 23001-7:2016(E), section 9.6.1:
    // "When the fields default_crypt_byte_block and default_skip_byte_block
    // in a version 1 Track Encryption Box ('tenc') are non-zero numbers,
    // pattern encryption SHALL be applied."
    // So for the pattern 0:0, assume that all blocks are encrypted.
    crypt_byte_block = total_blocks;
  }

  // Apply the pattern to |input_data|.
  // Example (using Pattern(2,3), Ex is encrypted, Ux unencrypted)
  //   input_data:  |E1|E2|U3|U4|U5|E6|E7|U8|U9|U10|E11|
  // We must decrypt 2 blocks, then simply copy the next 3 blocks, and
  // repeat until the end. Note that the input does not have to contain
  // a full pattern or even |crypt_byte_block| blocks at the end.
  size_t blocks_processed = 0;
  const uint8_t* src = input_data.data();
  uint8_t* dest = output_data;
  bool is_encrypted_blocks = false;
  while (blocks_processed < total_blocks) {
    is_encrypted_blocks = !is_encrypted_blocks;
    size_t blocks_to_process =
        std::min(is_encrypted_blocks ? crypt_byte_block : skip_byte_block,
                 total_blocks - blocks_processed);

    if (blocks_to_process == 0)
      continue;

    size_t bytes_to_process = blocks_to_process * kAesBlockSizeInBytes;

    // From ISO/IEC 23001-7:2016(E), section 10.4.2:
    // For a typical pattern length of 10 (e.g. 1:9) "the pattern is repeated
    // every 160 bytes of the protected range, until the end of the range. If
    // the protected range of the slice body is not a multiple of the pattern
    // length (e.g. 160 bytes), then the pattern sequence applies to the
    // included whole 16-byte Blocks and a partial 16-byte Block that may
    // remain where the pattern is terminated by the byte length of the range
    // BytesOfProtectedData, is left unencrypted."
    if (is_encrypted_blocks) {
      if (!aes_cbc_crypto.Decrypt(base::make_span(src, bytes_to_process),
                                  dest)) {
        return false;
      }
    } else {
      memcpy(dest, src, bytes_to_process);
    }

    blocks_processed += blocks_to_process;
    src += bytes_to_process;
    dest += bytes_to_process;
  }

  // Any partial block data remaining in this subsample is considered
  // unencrypted so simply copy it into |dest|.
  if (remaining_bytes > 0)
    memcpy(dest, src, remaining_bytes);

  return true;
}
```

위의 CENC, CBCS 코드를 요약하면 다음과 같습니다.

- CENC
    1. AES-CTR init(key=fnParam, iv=buffer.iv)
    2. Decrypt(encrypted_subsample_data)
- CBCS
    1. AES-CBC init(key=fnParam, iv=buffer.iv)
    2. Decrypt_Foreach_with_pattern(encrypted_subsamples_data)

여기서 관심을 가져야할 것은 `DecoderBuffer가 어떤식으로 만들어지는지, 어떻게 전달되는지` 입니다. 다음으로는 DecoderBuffer가 어디서 생성되는지 알아보도록 하겠습니다.

### 1.1.3 DecoderBuffer

아래의 그림과 같은 구조를 가진 chromium에서는 Browser와 Render 프로세스 간의 IPC 통신으로 메세지 필터링 후, 메세지에 알맞은 Renderer에 전달하게 됩니다.

![/assets/202102/202102won2.png](/assets/202102/202102won2.png)

즉, Renderer에서 Audio/Video Packet을 처리를 위한 Stream을 관리가 필요하므로 내부적으로 DecoderBuffer를 생성해야함을 알 수 있습니다. 저는 Audio와 관련된 Renderer에 관심을 가졌고 audio stream을 가져오는 초기화 과정부터 살펴봤습니다.

```cpp
// ./src/media/renderers/renderer_impl.cc
void RendererImpl::InitializeAudioRenderer() {
  DVLOG(1) << __func__;
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK_EQ(state_, STATE_INITIALIZING);
  DCHECK(init_cb_);

  DemuxerStream* audio_stream =
      media_resource_->GetFirstStream(DemuxerStream::AUDIO);

  if (!audio_stream) {
    audio_renderer_.reset();
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&RendererImpl::OnAudioRendererInitializeDone,
                                  weak_this_, PIPELINE_OK));
    return;
  }

  current_audio_stream_ = audio_stream;

  audio_renderer_client_.reset(
      new RendererClientInternal(DemuxerStream::AUDIO, this, media_resource_));
  **audio_renderer_->Initialize(
      audio_stream, cdm_context_, audio_renderer_client_.get(),
      base::BindOnce(&RendererImpl::OnAudioRendererInitializeDone, weak_this_));**
}
```

위와 같이 media_resource에서 stream을 획득하고 해당 audio stream을 audio renderer에 설정해줍니다.

```cpp
// ./src/media/renderers/renderer_impl.cc
void AudioRendererImpl::Initialize(DemuxerStream* stream,
                                   CdmContext* cdm_context,
                                   RendererClient* client,
                                   PipelineStatusCallback init_cb) {
  DVLOG(1) << __func__;
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK(client);
  DCHECK(stream);
  DCHECK_EQ(stream->type(), DemuxerStream::AUDIO);
  DCHECK(init_cb);
  DCHECK(state_ == kUninitialized || state_ == kFlushed);
  DCHECK(sink_);
  TRACE_EVENT_ASYNC_BEGIN0("media", "AudioRendererImpl::Initialize", this);

  if (state_ == kFlushed) {
    sink_->Stop();
    if (null_sink_)
      null_sink_->Stop();
  }

  state_ = kInitializing;
  demuxer_stream_ = stream;
  client_ = client;

  init_cb_ = BindToCurrentLoop(std::move(init_cb));

  **sink_->GetOutputDeviceInfoAsync(
      base::BindOnce(&AudioRendererImpl::OnDeviceInfoReceived,
                     weak_factory_.GetWeakPtr(), demuxer_stream_, cdm_context));**

#if !defined(OS_ANDROID)
  if (speech_recognition_client_) {
    speech_recognition_client_->SetOnReadyCallback(BindToCurrentLoop(
        base::BindOnce(&AudioRendererImpl::EnableSpeechRecognition,
                       weak_factory_.GetWeakPtr())));
  }
#endif
}
```

초기화 과정에서는 Audio Ouput Device 정보를 획득하고 OnDeviceInfoReceived 콜백함수를 호출하는 걸 볼 수 있습니다.

```cpp
// ./src/media/renderers/renderer_impl.cc
void AudioRendererImpl::OnDeviceInfoReceived(
    DemuxerStream* stream,
    CdmContext* cdm_context,
    OutputDeviceInfo output_device_info) {
  DVLOG(1) << __func__;
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK(client_);
  DCHECK(stream);
  DCHECK_EQ(stream->type(), DemuxerStream::AUDIO);
  DCHECK(init_cb_);
  DCHECK_EQ(state_, kInitializing);

  //...

  current_decoder_config_ = stream->audio_decoder_config();
  DCHECK(current_decoder_config_.IsValidConfig());

  const AudioParameters& hw_params = output_device_info.output_params();
  ChannelLayout hw_channel_layout =
      hw_params.IsValid() ? hw_params.channel_layout() : CHANNEL_LAYOUT_NONE;

  audio_decoder_stream_ = std::make_unique<AudioDecoderStream>(
      std::make_unique<AudioDecoderStream::StreamTraits>(media_log_,
                                                         hw_channel_layout),
      task_runner_, create_audio_decoders_cb_, media_log_);

  audio_decoder_stream_->set_config_change_observer(base::BindRepeating(
      &AudioRendererImpl::OnConfigChange, weak_factory_.GetWeakPtr()));

  AudioCodec codec = stream->audio_decoder_config().codec();
  if (auto* mc = GetMediaClient())
    is_passthrough_ = mc->IsSupportedBitstreamAudioCodec(codec);
  else
    is_passthrough_ = false;
  expecting_config_changes_ = stream->SupportsConfigChanges();

  bool use_stream_params = !expecting_config_changes_ || !hw_params.IsValid() ||
                           hw_params.format() == AudioParameters::AUDIO_FAKE ||
                           !sink_->IsOptimizedForHardwareParameters();

  if (stream->audio_decoder_config().channel_layout() ==
          CHANNEL_LAYOUT_DISCRETE &&
      sink_->IsOptimizedForHardwareParameters()) {
    use_stream_params = false;
  }

  const int preferred_buffer_size =
      std::max(2 * stream->audio_decoder_config().samples_per_second() / 100,
               hw_params.IsValid() ? hw_params.frames_per_buffer() : 0);

  if (is_passthrough_) {
    AudioParameters::Format format = AudioParameters::AUDIO_FAKE;
    if (codec == kCodecAC3) {
      format = AudioParameters::AUDIO_BITSTREAM_AC3;
    } else if (codec == kCodecEAC3) {
      format = AudioParameters::AUDIO_BITSTREAM_EAC3;
    } else {
      NOTREACHED();
    }

    const int buffer_size =
        AudioParameters::kMaxFramesPerCompressedAudioBuffer *
        stream->audio_decoder_config().bytes_per_frame();

    audio_parameters_.Reset(
        format, stream->audio_decoder_config().channel_layout(),
        stream->audio_decoder_config().samples_per_second(), buffer_size);
    buffer_converter_.reset();
  } else if (use_stream_params) {
    audio_parameters_.Reset(AudioParameters::AUDIO_PCM_LOW_LATENCY,
                            stream->audio_decoder_config().channel_layout(),
                            stream->audio_decoder_config().samples_per_second(),
                            preferred_buffer_size);
    audio_parameters_.set_channels_for_discrete(
        stream->audio_decoder_config().channels());
    buffer_converter_.reset();
  } else {
    int sample_rate = hw_params.sample_rate();

    if (AudioLatency::IsResamplingPassthroughSupported(
            AudioLatency::LATENCY_PLAYBACK) &&
        stream->audio_decoder_config().samples_per_second() >= 44100) {
      sample_rate = stream->audio_decoder_config().samples_per_second();
    }

    int stream_channel_count = stream->audio_decoder_config().channels();

    bool try_supported_channel_layouts = false;
#if defined(OS_WIN)
    try_supported_channel_layouts =
        base::CommandLine::ForCurrentProcess()->HasSwitch(
            switches::kTrySupportedChannelLayouts);
#endif

    ChannelLayout hw_channel_layout =
        hw_params.channel_layout() == CHANNEL_LAYOUT_DISCRETE ||
                try_supported_channel_layouts
            ? CHANNEL_LAYOUT_STEREO
            : hw_params.channel_layout();
    int hw_channel_count = ChannelLayoutToChannelCount(hw_channel_layout);

    ChannelLayout renderer_channel_layout =
        hw_channel_count > stream_channel_count
            ? hw_channel_layout
            : stream->audio_decoder_config().channel_layout();

    audio_parameters_.Reset(hw_params.format(), renderer_channel_layout,
                            sample_rate,
                            AudioLatency::GetHighLatencyBufferSize(
                                sample_rate, preferred_buffer_size));
  }

  audio_parameters_.set_effects(audio_parameters_.effects() |
                                AudioParameters::MULTIZONE);

  audio_parameters_.set_latency_tag(AudioLatency::LATENCY_PLAYBACK);

  if (!client_->IsVideoStreamAvailable()) {
    audio_parameters_.set_effects(audio_parameters_.effects() |
                                  AudioParameters::AUDIO_PREFETCH);
  }

  last_decoded_channel_layout_ =
      stream->audio_decoder_config().channel_layout();

  is_encrypted_ = stream->audio_decoder_config().is_encrypted();

  last_decoded_channels_ = stream->audio_decoder_config().channels();

  {
    base::AutoLock lock(lock_);
    audio_clock_.reset(
        new AudioClock(base::TimeDelta(), audio_parameters_.sample_rate()));
  }

  **audio_decoder_stream_->Initialize(
      stream,
      base::BindOnce(&AudioRendererImpl::OnAudioDecoderStreamInitialized,
                     weak_factory_.GetWeakPtr()),
      cdm_context,
      base::BindRepeating(&AudioRendererImpl::OnStatisticsUpdate,
                          weak_factory_.GetWeakPtr()),
      base::BindRepeating(&AudioRendererImpl::OnWaiting,
                          weak_factory_.GetWeakPtr()));**
}
```

OnDeviceInfoReceived 함수는 Output Device가 재생가능한 상태인지 확인한 이후, Audio의 정보(ex. is_encrypted)를 Renderer의 멤버 변수에 설정합니다. 또한 Decoder Stream을 초기화 진행합니다. 이제 초기화가 완료되었고, 해당 Stream에서 Buffer를 사용할 것으로 보이므로 해당 DecodeStream을 추가 분석했습니다.

```cpp
// ./src/media/filters/decoder_stream.h
using AudioDecoderStream = DecoderStream<DemuxerStream::AUDIO>;

// ./src/media/filters/decoder_stream.cc
template <DemuxerStream::Type StreamType>
void DecoderStream<StreamType>::Initialize(DemuxerStream* stream,
                                           InitCB init_cb,
                                           CdmContext* cdm_context,
                                           StatisticsCB statistics_cb,
                                           WaitingCB waiting_cb) {
  FUNCTION_DVLOG(1);
  DCHECK(task_runner_->RunsTasksInCurrentSequence());
  DCHECK_EQ(state_, STATE_UNINITIALIZED);
  DCHECK(!init_cb_);
  DCHECK(init_cb);

  stream_ = stream;
  init_cb_ = std::move(init_cb);
  cdm_context_ = cdm_context;
  statistics_cb_ = std::move(statistics_cb);

  // Make a copy here since it's also passed to |decoder_selector_| below.
  waiting_cb_ = waiting_cb;

  traits_->OnStreamReset(stream_);
  decoder_selector_.Initialize(traits_.get(), stream, cdm_context,
                               std::move(waiting_cb));

  state_ = STATE_INITIALIZING;
  **SelectDecoder();**
}

template <DemuxerStream::Type StreamType>
void DecoderSelector<StreamType>::SelectDecoder(
    SelectDecoderCB select_decoder_cb,
    typename Decoder::OutputCB output_cb) {
  DVLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(select_decoder_cb);
  DCHECK(!select_decoder_cb_);
  select_decoder_cb_ = std::move(select_decoder_cb);
  output_cb_ = std::move(output_cb);
  config_ = traits_->GetDecoderConfig(stream_);

  TRACE_EVENT_ASYNC_BEGIN2("media", kSelectDecoderTrace, this, "type",
                           DemuxerStream::GetTypeName(StreamType), "config",
                           config_.AsHumanReadableString());

  if (!config_.IsValidConfig()) {
    DLOG(ERROR) << "Invalid stream config";
    ReturnNullDecoder();
    return;
  }

  // If this is the first selection (ever or since FinalizeDecoderSelection()),
  // start selection with the full list of potential decoders.
  if (!is_selecting_decoders_) {
    is_selecting_decoders_ = true;
    decoder_selection_start_ = base::TimeTicks::Now();
    CreateDecoders();
  }

  **InitializeDecoder();**
}
```

위의 코드에서 볼 수 있듯, DecoderStream를 초기화할 때 Decoder를 선택하는 것을 볼 수 있습니다. 그리고 선택된 Decoder를 초기화하는 것을 볼 수 있는데, 해당 함수를 따라가다보면 아래의 함수를 볼 수 있습니다.

```cpp
template <DemuxerStream::Type StreamType>
void DecoderSelector<StreamType>::InitializeDecoder() {
  DVLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!decoder_);

  if (decoders_.empty()) {
    // Decoder selection failed. If the stream is encrypted, try again using
    // DecryptingDemuxerStream.
    if (config_.is_encrypted() && cdm_context_) {
      InitializeDecryptingDemuxerStream();
      return;
    }

    ReturnNullDecoder();
    return;
  }
  // ...
}
```

decoders 변수가 비어있을 경우, encrypted media인 것을 볼 수 있고 Decrypt기능이 포함된 DemuxerStream을 초기화합니다.  그러므로 이후에 media 데이터를 demux할 때, DecryptingDemuxerStream을 사용해서 decrypt 처리를 진행한다는 것을 알 수 있습니다. 따라서, 해당 클래스가 Demux시 사용하는 함수를 분석해야합니다. 아래는 해당 코드입니다.

```cpp
// ./src/media/filters/decrypting_demuxer_stream.cc
void DecryptingDemuxerStream::Read(ReadCB read_cb) {
  DVLOG(3) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, kIdle) << state_;
  DCHECK(read_cb);
  CHECK(!read_cb_) << "Overlapping reads are not supported.";

  read_cb_ = BindToCurrentLoop(std::move(read_cb));
  state_ = kPendingDemuxerRead;
  **demuxer_stream_->Read(
      base::BindOnce(&DecryptingDemuxerStream::OnBufferReadFromDemuxerStream,
                     weak_factory_.GetWeakPtr()));**
}

void DecryptingDemuxerStream::OnBufferReadFromDemuxerStream(
    DemuxerStream::Status status,
    scoped_refptr<DecoderBuffer> buffer) {
  DVLOG(3) << __func__ << ": status = " << status;
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, kPendingDemuxerRead) << state_;
  DCHECK(read_cb_);
  DCHECK_EQ(buffer.get() != nullptr, status == kOk) << status;

  // Even when |reset_cb_|, we need to pass |kConfigChanged| back to
  // the caller so that the downstream decoder can be properly reinitialized.
  if (status == kConfigChanged) {
    DVLOG(2) << __func__ << ": config change";
    DCHECK_EQ(demuxer_stream_->type() == AUDIO, audio_config_.IsValidConfig());
    DCHECK_EQ(demuxer_stream_->type() == VIDEO, video_config_.IsValidConfig());

    // Update the decoder config, which the decoder will use when it is notified
    // of kConfigChanged.
    InitializeDecoderConfig();

    state_ = kIdle;
    std::move(read_cb_).Run(kConfigChanged, nullptr);
    if (reset_cb_)
      DoReset();
    return;
  }

  if (reset_cb_) {
    std::move(read_cb_).Run(kAborted, nullptr);
    DoReset();
    return;
  }

  if (status == kAborted || status == kError) {
    if (status == kError) {
      MEDIA_LOG(ERROR, media_log_)
          << GetDisplayName() << ": demuxer stream read error.";
    }
    state_ = kIdle;
    std::move(read_cb_).Run(status, nullptr);
    return;
  }

  DCHECK_EQ(kOk, status);

  if (buffer->end_of_stream()) {
    DVLOG(2) << __func__ << ": EOS buffer";
    state_ = kIdle;
    std::move(read_cb_).Run(kOk, std::move(buffer));
    return;
  }

  if (!buffer->decrypt_config()) {
    DVLOG(2) << __func__ << ": clear buffer";
    state_ = kIdle;
    std::move(read_cb_).Run(kOk, std::move(buffer));
    return;
  }

  pending_buffer_to_decrypt_ = std::move(buffer);
  state_ = kPendingDecrypt;
  **DecryptPendingBuffer();**
}

void DecryptingDemuxerStream::DecryptPendingBuffer() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, kPendingDecrypt) << state_;
  DCHECK(!pending_buffer_to_decrypt_->end_of_stream());
  TRACE_EVENT_ASYNC_BEGIN2(
      "media", "DecryptingDemuxerStream::DecryptPendingBuffer", this, "type",
      DemuxerStream::GetTypeName(demuxer_stream_->type()), "timestamp_us",
      pending_buffer_to_decrypt_->timestamp().InMicroseconds());
  **decryptor_->Decrypt(GetDecryptorStreamType(), pending_buffer_to_decrypt_,
                      BindToCurrentLoop(base::BindOnce(
                          &DecryptingDemuxerStream::OnBufferDecrypted,
                          weak_factory_.GetWeakPtr())));**
}
```

위의 코드에서 알 수 있듯, Demux를 진행하기에 앞서 데이터를 읽는 부분에서 DecoderBuffer를 사용하는 것을 볼 수 있고, 읽어들인 Buffer를 Decrypt까지 하는 것을 확인할 수 있습니다.

따라서, DecoderBuffer가 처리되는 흐름을 요약하면 다음과 같습니다.

AudioRenderer reset & init → Output Device Status check → DecoderStream init → DemuxerStream init → Encrypted media check → DecryptingDemuxerStream init → ready to play → DecryptingDemuxerStream Read → read status check → Decrypt buffer & demux

다음 문서는 동적 분석을 통한 Chrome EME Drm 해제 가능 여부에 대해서 알아보도록 하겠습니다.

## Reference

1. EME specification

    [https://w3.org/TR/encrypted-media](https://w3.org/TR/encrypted-media)

2. Common Encryption (CENC)

    [https://source.chromium.org/chromium/chromium/src/+/master:media/cdm/cenc_decryptor.cc;l=62;bpv=0;bpt=0](https://source.chromium.org/chromium/chromium/src/+/master:media/cdm/cenc_decryptor.cc;l=62;bpv=0;bpt=0)

3. CBCS(AES-CBC CENC)

    [https://source.chromium.org/chromium/chromium/src/+/master:media/cdm/cbcs_decryptor.cc;l=121;bpv=0;bpt=0](https://source.chromium.org/chromium/chromium/src/+/master:media/cdm/cbcs_decryptor.cc;l=121;bpv=0;bpt=0)

4. DecoderBuffer

    [https://source.chromium.org/chromium/chromium/src/+/master:media/base/decoder_buffer.h;l=34;drc=1febd4367152c37a707817cbc25a1b88c527b6bb](https://source.chromium.org/chromium/chromium/src/+/master:media/base/decoder_buffer.h;l=34;drc=1febd4367152c37a707817cbc25a1b88c527b6bb)