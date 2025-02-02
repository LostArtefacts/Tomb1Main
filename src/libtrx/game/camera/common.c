static bool m_IsChunky = false;

bool Camera_IsChunky(void)
{
    return m_IsChunky;
}

void Camera_SetChunky(const bool is_chunky)
{
    m_IsChunky = is_chunky;
}
