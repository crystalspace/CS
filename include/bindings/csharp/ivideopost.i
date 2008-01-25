/* ivideo specifics extends */
#ifdef SWIGCSHARP

%extend iGraphics2D
{
  void Blit(int x, int y, int width, int height, iDataBuffer *data)
  {
    self->Blit(x, y, width, height, (unsigned char const *)data->GetData());
  }

  void Blit(int x, int y, int width, int height, void *data)
  {
    self->Blit(x, y, width, height, (unsigned char const *)data);
  }
}

#endif
