module Editor.SnapshotFrame;

Editor::SnapshotPublisher::SnapshotPublisher(SnapshotPublisher&& other) noexcept
    : m_frames{std::move(other.m_frames)},
      m_readIndex{std::move(other.m_readIndex.load())},
      m_writeIndex{std::move(other.m_writeIndex)} {}

Editor::SnapshotPublisher& Editor::SnapshotPublisher::operator=(SnapshotPublisher&& other) noexcept
{
    m_frames = std::move(other.m_frames);
    m_readIndex = std::move(other.m_readIndex.load());
    m_writeIndex = std::move(other.m_writeIndex);
    return *this;
}
