#ifndef MOCK_H
#define MOCK_H
static void mockPopulatePackages(std::string* response)
{
  response->append(std::string(R"MOCK({
    "packages": [
      {
        "name": "example-1",
        "title": "Example One",
        "author": "vgmoose",
        "description": "An example of things",
        "details": "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent tristique, velit id venenatis congue, tortor ex fermentum ligula, a accumsan mauris justo quis ipsum. In hac habitasse platea dictumst. Interdum et malesuada fames ac ante ipsum primis in faucibus. Integer ut odio non arcu aliquet molestie a rhoncus risus. Vivamus at magna congue, consequat ligula quis, tempus eros. Aliquam eleifend consectetur arcu ornare elementum. Integer eu ipsum mi. Nulla vitae elit quis leo vulputate condimentum sit amet in dui. Etiam venenatis condimentum ex et mattis. Donec mollis facilisis dapibus.\\\\n\\\\nCras rhoncus, nunc quis hendrerit rhoncus, velit justo aliquam dolor, eget vestibulum nisi ligula non risus. Fusce nulla nisi, dignissim ac facilisis et, dignissim et eros. Integer pellentesque purus non blandit consectetur. Duis id commodo ante, in molestie sapien. Integer molestie ultrices enim, eu commodo magna lobortis ut. Nullam sollicitudin eleifend venenatis. Phasellus lacinia, dolor id hendrerit venenatis, nunc orci sollicitudin ipsum, malesuada lacinia urna massa in turpis. Cras nec lacus id arcu hendrerit consectetur a sit amet tellus. Sed rutrum malesuada ipsum, sit amet malesuada risus interdum eu. Sed placerat quam nec ex facilisis porta sed vehicula ante. Donec imperdiet sem ut purus luctus, at venenatis leo finibus."
      },
      {
        "name": "example-2",
        "title": "Another Example",
        "author": "pwsincd",
        "description": "More examples of things",
        "details": "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent tristique, velit id venenatis congue, tortor ex fermentum ligula, a accumsan mauris justo quis ipsum. In hac habitasse platea dictumst. Interdum et malesuada fames ac ante ipsum primis in faucibus. Integer ut odio non arcu aliquet molestie a rhoncus risus. Vivamus at magna congue, consequat ligula quis, tempus eros. Aliquam eleifend consectetur arcu ornare elementum. Integer eu ipsum mi. Nulla vitae elit quis leo vulputate condimentum sit amet in dui. Etiam venenatis condimentum ex et mattis. Donec mollis facilisis dapibus.\\\\n\\\\nCras rhoncus, nunc quis hendrerit rhoncus, velit justo aliquam dolor, eget vestibulum nisi ligula non risus. Fusce nulla nisi, dignissim ac facilisis et, dignissim et eros. Integer pellentesque purus non blandit consectetur. Duis id commodo ante, in molestie sapien. Integer molestie ultrices enim, eu commodo magna lobortis ut. Nullam sollicitudin eleifend venenatis. Phasellus lacinia, dolor id hendrerit venenatis, nunc orci sollicitudin ipsum, malesuada lacinia urna massa in turpis. Cras nec lacus id arcu hendrerit consectetur a sit amet tellus. Sed rutrum malesuada ipsum, sit amet malesuada risus interdum eu. Sed placerat quam nec ex facilisis porta sed vehicula ante. Donec imperdiet sem ut purus luctus, at venenatis leo finibus."
      },
      { "name": "test1" },
      { "name": "test2" },
      { "name": "test3" },
      { "name": "test4" },
      { "name": "test5" },
      { "name": "test6" },
      { "name": "test7" },
      { "name": "test8" },
      { "name": "test9" },
      { "name": "test10" },
      { "name": "test11" },
      { "name": "test12" },
      { "name": "test13" },
      { "name": "test14" },
      { "name": "test15" },
      { "name": "test16" },
      { "name": "test17" },
      { "name": "test18" },
      { "name": "test19" },
      { "name": "test20" }
    ]
  })MOCK"));
}
#endif