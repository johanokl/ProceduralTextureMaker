#include <utility>

// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturerenderthread.h"
#include "global.h"
#include "texturenode.h"

TextureRenderThread::TextureRenderThread(const QSize inRendersize,
                                         QMap<int, TextureNodePtr> inNodesMap)
    : renderSize(inRendersize), nodes(std::move(inNodesMap)), aborted(false) {}

void TextureRenderThread::abort() { aborted.store(true); }

void TextureRenderThread::generate() {
   bool someGenerated;
   do {
      someGenerated = false;
      QMapIterator<int, TextureNodePtr> nodeIterator(nodes);
      while (!aborted.load() && nodeIterator.hasNext()) {
         TextureNodePtr currNode = nodeIterator.next().value();
         if (currNode->waitingFor(renderSize) == 0 && !currNode->isTextureInCache(renderSize)) {
            currNode->getImage(renderSize);
            someGenerated = true;
         }
      }
   } while (!aborted.load() && someGenerated);
}

void TextureRenderThread::imageUpdated() { generate(); }

void TextureRenderThread::nodeAdded(const TextureNodePtr& newNode) {
   if (!nodes.contains(newNode->getId())) {
      nodes.insert(newNode->getId(), newNode);
      generate();
   }
}

void TextureRenderThread::nodeRemoved(int id) {
   if (nodes.contains(id)) {
      nodes.remove(id);
      generate();
   }
}
