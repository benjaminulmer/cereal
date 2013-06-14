#include <cereal/binary_archive/binary_archive.hpp>
#include <cereal/binary_archive/memory.hpp>
#include <cereal/binary_archive/array.hpp>
#include <cereal/binary_archive/vector.hpp>
#include <cereal/binary_archive/deque.hpp>
#include <limits>
#include <random>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Cereal
#include <boost/test/unit_test.hpp>

struct StructBase
{
  int x, y;
  bool operator==(StructBase const & other)
  { return x == other.x && y == other.y; }
  bool operator!=(StructBase const & other)
  { return x != other.x || y != other.y; }
};

std::ostream& operator<<(std::ostream& os, StructBase const & s)
{
    os << "[x: " << s.x << " y: " << s.y << "]";
    return os;
}

 
struct StructInternalSerialize : StructBase
{
  StructInternalSerialize() : StructBase{0,0} {}
  StructInternalSerialize(int x_, int y_) : StructBase{x_,y_} {} 
  template<class Archive>
    void serialize(Archive & ar)
    {
      ar & x & y;
    }
};

struct StructInternalSplit : StructBase
{
  StructInternalSplit() : StructBase{0,0} {}
  StructInternalSplit(int x_, int y_) : StructBase{x_,y_} {} 
  template<class Archive>
    void save(Archive & ar)
    {
      ar & x & y;
    }

  template<class Archive>
    void load(Archive & ar)
    {
      ar & x & y;
    }
};

struct StructExternalSerialize : StructBase
{
  StructExternalSerialize() : StructBase{0,0} {}
  StructExternalSerialize(int x_, int y_) : StructBase{x_,y_} {} 
};

  template<class Archive>
void serialize(Archive & ar, StructExternalSerialize & s)
{
  ar & s.x & s.y;
}

struct StructExternalSplit : StructBase
{
  StructExternalSplit() : StructBase{0,0} {}
  StructExternalSplit(int x_, int y_) : StructBase{x_,y_} {} 
};

  template<class Archive>
void save(Archive & ar, StructExternalSplit & s)
{
  ar & s.x & s.y;
}

  template<class Archive>
void load(Archive & ar, StructExternalSplit & s)
{
  ar & s.x & s.y;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
random_value(std::mt19937 & gen)
{ return std::uniform_real_distribution<T>(-10000.0, 10000.0)(gen); }

template<class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
random_value(std::mt19937 & gen)
{ return std::uniform_int_distribution<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())(gen); }

// ######################################################################
BOOST_AUTO_TEST_CASE( binary_pod )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(size_t i=0; i<100; ++i)
  {
    uint8_t  const o_uint8  = random_value<uint8_t>(gen);
    int8_t   const o_int8   = random_value<int8_t>(gen);
    uint16_t const o_uint16 = random_value<uint16_t>(gen);
    int16_t  const o_int16  = random_value<int16_t>(gen);
    uint32_t const o_uint32 = random_value<uint32_t>(gen);
    int32_t  const o_int32  = random_value<int32_t>(gen);
    uint64_t const o_uint64 = random_value<uint64_t>(gen);
    int64_t  const o_int64  = random_value<int64_t>(gen);
    float    const o_float  = random_value<float>(gen);
    double   const o_double = random_value<double>(gen);

    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);
    oar & o_uint8;
    oar & o_int8;
    oar & o_uint16;
    oar & o_int16;
    oar & o_uint32;
    oar & o_int32;
    oar & o_uint64;
    oar & o_int64;
    oar & o_float;
    oar & o_double;

    uint8_t  i_uint8  = 0.0;
    int8_t   i_int8   = 0.0;
    uint16_t i_uint16 = 0.0;
    int16_t  i_int16  = 0.0;
    uint32_t i_uint32 = 0.0;
    int32_t  i_int32  = 0.0;
    uint64_t i_uint64 = 0.0;
    int64_t  i_int64  = 0.0;
    float    i_float  = 0.0;
    double   i_double = 0.0;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);
    iar & i_uint8;
    iar & i_int8;
    iar & i_uint16;
    iar & i_int16;
    iar & i_uint32;
    iar & i_int32;
    iar & i_uint64;
    iar & i_int64;
    iar & i_float;
    iar & i_double;

    BOOST_CHECK_EQUAL(i_uint8  , o_uint8);
    BOOST_CHECK_EQUAL(i_int8   , o_int8);
    BOOST_CHECK_EQUAL(i_uint16 , o_uint16);
    BOOST_CHECK_EQUAL(i_int16  , o_int16);
    BOOST_CHECK_EQUAL(i_uint32 , o_uint32);
    BOOST_CHECK_EQUAL(i_int32  , o_int32);
    BOOST_CHECK_EQUAL(i_uint64 , o_uint64);
    BOOST_CHECK_EQUAL(i_int64  , o_int64);
    BOOST_CHECK_EQUAL(i_float  , o_float);
    BOOST_CHECK_EQUAL(i_double , o_double);
  }
}

// ######################################################################
BOOST_AUTO_TEST_CASE( structs )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(int i=0; i<100; ++i)
  {
    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);

    StructInternalSerialize o_iser = { random_value<int>(gen), random_value<int>(gen) };
    StructInternalSplit     o_ispl = { random_value<int>(gen), random_value<int>(gen) };
    StructExternalSerialize o_eser = { random_value<int>(gen), random_value<int>(gen) };
    StructExternalSplit     o_espl = { random_value<int>(gen), random_value<int>(gen) };

    oar & o_iser & o_ispl & o_eser & o_espl;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);

    StructInternalSerialize i_iser;
    StructInternalSplit     i_ispl;
    StructExternalSerialize i_eser;
    StructExternalSplit     i_espl;

    iar & i_iser & i_ispl & i_eser & i_espl;
  
    BOOST_CHECK(i_iser == o_iser);
    BOOST_CHECK(i_ispl == o_ispl);
    BOOST_CHECK(i_eser == o_eser);
    BOOST_CHECK(i_espl == o_espl);
  }
}

// ######################################################################
BOOST_AUTO_TEST_CASE( binary_memory )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(int i=0; i<100; ++i)
  {
    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);

    std::shared_ptr<int> o_xptr1 = std::make_shared<int>(random_value<int>(gen));
    std::shared_ptr<int> o_xptr2 = o_xptr1;
    std::shared_ptr<int> o_yptr1 = std::make_shared<int>(random_value<int>(gen));
    std::shared_ptr<int> o_yptr2 = o_yptr1;
    oar & o_xptr1 & o_xptr2;
    oar & o_yptr1 & o_yptr2;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);

    std::shared_ptr<int> i_xptr1;
    std::shared_ptr<int> i_xptr2;
    std::shared_ptr<int> i_yptr1;
    std::shared_ptr<int> i_yptr2;
    iar & i_xptr1 & i_xptr2;
    iar & i_yptr1 & i_yptr2;

    BOOST_CHECK_EQUAL(o_xptr1.get(), o_xptr2.get());
    BOOST_CHECK_EQUAL(i_xptr1.get(), i_xptr2.get());
    BOOST_CHECK_EQUAL(*i_xptr1, *i_xptr2);

    BOOST_CHECK_EQUAL(o_yptr1.get(), o_yptr2.get());
    BOOST_CHECK_EQUAL(i_yptr1.get(), i_yptr2.get());
    BOOST_CHECK_EQUAL(*i_yptr1, *i_yptr2);
  }
}

// ######################################################################
BOOST_AUTO_TEST_CASE( binary_array )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(int i=0; i<100; ++i)
  {
    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);

    std::array<int, 100> o_podarray;
    for(auto & elem : o_podarray)
      elem = random_value<decltype(o_podarray)::value_type>(gen);

    std::array<StructInternalSerialize, 100> o_iserarray;
    for(auto & elem : o_iserarray)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::array<StructInternalSplit, 100> o_isplarray;
    for(auto & elem : o_isplarray)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::array<StructExternalSerialize, 100> o_eserarray;
    for(auto & elem : o_eserarray)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::array<StructExternalSplit, 100> o_esplarray;
    for(auto & elem : o_esplarray)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    oar & o_podarray;
    oar & o_iserarray;
    oar & o_isplarray;
    oar & o_eserarray;
    oar & o_esplarray;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);

    std::array<int, 100> i_podarray;
    std::array<StructInternalSerialize, 100> i_iserarray;
    std::array<StructInternalSplit, 100>     i_isplarray;
    std::array<StructExternalSerialize, 100> i_eserarray;
    std::array<StructExternalSplit, 100>     i_esplarray;

    iar & i_podarray;
    iar & i_iserarray;
    iar & i_isplarray;
    iar & i_eserarray;
    iar & i_esplarray;

    BOOST_CHECK_EQUAL_COLLECTIONS(i_podarray.begin(), i_podarray.end(), o_podarray.begin(), o_podarray.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_iserarray.begin(), i_iserarray.end(), o_iserarray.begin(), o_iserarray.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_isplarray.begin(), i_isplarray.end(), o_isplarray.begin(), o_isplarray.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_eserarray.begin(), i_eserarray.end(), o_eserarray.begin(), o_eserarray.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_esplarray.begin(), i_esplarray.end(), o_esplarray.begin(), o_esplarray.end());
  }
}

// ######################################################################
BOOST_AUTO_TEST_CASE( binary_vector )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(int i=0; i<100; ++i)
  {
    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);

    std::vector<int> o_podvector(100);
    for(auto & elem : o_podvector)
      elem = random_value<decltype(o_podvector)::value_type>(gen);

    std::vector<StructInternalSerialize> o_iservector(100);
    for(auto & elem : o_iservector)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::vector<StructInternalSplit> o_isplvector(100);
    for(auto & elem : o_isplvector)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::vector<StructExternalSerialize> o_eservector(100);
    for(auto & elem : o_eservector)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::vector<StructExternalSplit> o_esplvector(100);
    for(auto & elem : o_esplvector)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    oar & o_podvector;
    oar & o_iservector;
    oar & o_isplvector;
    oar & o_eservector;
    oar & o_esplvector;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);

    std::vector<int> i_podvector;
    std::vector<StructInternalSerialize> i_iservector;
    std::vector<StructInternalSplit>     i_isplvector;
    std::vector<StructExternalSerialize> i_eservector;
    std::vector<StructExternalSplit>     i_esplvector;

    iar & i_podvector;
    iar & i_iservector;
    iar & i_isplvector;
    iar & i_eservector;
    iar & i_esplvector;

    BOOST_CHECK_EQUAL(i_podvector.size(),  o_podvector.size());
    BOOST_CHECK_EQUAL(i_iservector.size(), o_iservector.size());
    BOOST_CHECK_EQUAL(i_isplvector.size(), o_isplvector.size());
    BOOST_CHECK_EQUAL(i_eservector.size(), o_eservector.size());
    BOOST_CHECK_EQUAL(i_esplvector.size(), o_esplvector.size());

    BOOST_CHECK_EQUAL_COLLECTIONS(i_podvector.begin(), i_podvector.end(), o_podvector.begin(), o_podvector.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_iservector.begin(), i_iservector.end(), o_iservector.begin(), o_iservector.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_isplvector.begin(), i_isplvector.end(), o_isplvector.begin(), o_isplvector.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_eservector.begin(), i_eservector.end(), o_eservector.begin(), o_eservector.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_esplvector.begin(), i_esplvector.end(), o_esplvector.begin(), o_esplvector.end());
  }
}

// ######################################################################
BOOST_AUTO_TEST_CASE( binary_deque )
{
  std::random_device rd;
  std::mt19937 gen(rd());

  for(int i=0; i<100; ++i)
  {
    std::ostringstream os;
    cereal::BinaryOutputArchive oar(os);

    std::deque<int> o_poddeque(100);
    for(auto & elem : o_poddeque)
      elem = random_value<decltype(o_poddeque)::value_type>(gen);

    std::deque<StructInternalSerialize> o_iserdeque(100);
    for(auto & elem : o_iserdeque)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::deque<StructInternalSplit> o_ispldeque(100);
    for(auto & elem : o_ispldeque)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::deque<StructExternalSerialize> o_eserdeque(100);
    for(auto & elem : o_eserdeque)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    std::deque<StructExternalSplit> o_espldeque(100);
    for(auto & elem : o_espldeque)
      elem = { random_value<int>(gen), random_value<int>(gen) };

    oar & o_poddeque;
    oar & o_iserdeque;
    oar & o_ispldeque;
    oar & o_eserdeque;
    oar & o_espldeque;

    std::istringstream is(os.str());
    cereal::BinaryInputArchive iar(is);

    std::deque<int> i_poddeque;
    std::deque<StructInternalSerialize> i_iserdeque;
    std::deque<StructInternalSplit>     i_ispldeque;
    std::deque<StructExternalSerialize> i_eserdeque;
    std::deque<StructExternalSplit>     i_espldeque;

    iar & i_poddeque;
    iar & i_iserdeque;
    iar & i_ispldeque;
    iar & i_eserdeque;
    iar & i_espldeque;

    BOOST_CHECK_EQUAL(i_poddeque.size(),  o_poddeque.size());
    BOOST_CHECK_EQUAL(i_iserdeque.size(), o_iserdeque.size());
    BOOST_CHECK_EQUAL(i_ispldeque.size(), o_ispldeque.size());
    BOOST_CHECK_EQUAL(i_eserdeque.size(), o_eserdeque.size());
    BOOST_CHECK_EQUAL(i_espldeque.size(), o_espldeque.size());

    BOOST_CHECK_EQUAL_COLLECTIONS(i_poddeque.begin(), i_poddeque.end(), o_poddeque.begin(), o_poddeque.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_iserdeque.begin(), i_iserdeque.end(), o_iserdeque.begin(), o_iserdeque.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_ispldeque.begin(), i_ispldeque.end(), o_ispldeque.begin(), o_ispldeque.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_eserdeque.begin(), i_eserdeque.end(), o_eserdeque.begin(), o_eserdeque.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_espldeque.begin(), i_espldeque.end(), o_espldeque.begin(), o_espldeque.end());
  }
}