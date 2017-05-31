# Copyright 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

prefix = /usr
bindir = $(prefix)/bin

CXXFLAGS += -std=c++11
OBJS= image_header.o create_image.o

create_image: $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

all: create_image

.PHONY: clean
clean:
	rm -rf $(OBJS) create_image

.PHONY: install
install: all
	install -d $(DESTDIR)$(bindir)
	install create_image $(DESTDIR)$(bindir)/create_image
