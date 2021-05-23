
template <typename T>
class Fifo
{
  public:
    Fifo(size_t maxSize): maxSize(maxSize)
    {
      this->clear();
    }

    void clear()
    {
      this->firstNode = 0;
      this->lastNode = 0;
      this->nofNodes = 0;
    }

    int add(T node)
    {
      if (!this->isFull())
      {
          (++this->lastNode) % this->maxSize;
          this->buffer[this->lastNode] = node;
          ++this->nofNodes;
          return 0;
      }
      
      return 1;
    }

    int pop(T &node)
    {
      if (!this->isEmpty())
      {
        node = this->buffer[this->firstNode];
        --this->nofNodes;
        (++this->firstNode) % this->maxSize;
        return 0;
      }

      return 1;

    }

    int isEmpty()
    {
      return this->nofNodes == 0;
    }

    int isFull()
    {
      return this->nofNodes == maxSize;
    }
    
  private:
    size_t firstNode;
    size_t lastNode;
    size_t nofNodes;
    const size_t maxSize;
    T buffer[maxSize];
};
