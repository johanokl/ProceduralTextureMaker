/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "texturerenderthread.h"
#include "texturenode.h"
#include "global.h"

/**
 * @brief TextureRenderThread::TextureRenderThread
 * @param renderSize Static rendersize.
 * @param nodes
 */
TextureRenderThread::TextureRenderThread(const QSize inRendersize, QMap<int, TextureNodePtr> inNodesMap)
   : renderSize(inRendersize), nodes(inNodesMap)
{
   aborted = false;
}

/**
 * @brief TextureRenderThread::abort
 *
 * Stops a thread from rendering more images.
 * Finishes the current one and then stops.
 */
void TextureRenderThread::abort()
{
   aborted = true;
}

/**
 * @brief TextureRenderThread::generate
 *
 * Loops through the node list and generates images until all nodes
 * have an image this thread's image size in their cache.
 */
void TextureRenderThread::generate()
{
   bool someGenerated;
   do {
      someGenerated = false;
      QMapIterator<int, TextureNodePtr> nodeIterator(nodes);
      while (!aborted && nodeIterator.hasNext()) {
         TextureNodePtr currNode = nodeIterator.next().value();
         if (currNode->waitingFor(renderSize) == 0
             && !currNode->isTextureInCache(renderSize)) {
            currNode->getImage(renderSize);
            someGenerated = true;
         }
      }
   } while (!aborted && someGenerated);
}

/**
 * @brief TextureRenderThread::imageUpdated
 * @param id Node id.
 *
 * Callback function for when a node has been updated
 * and a new image needs to be rendered for it.
 */
void TextureRenderThread::imageUpdated()
{
   generate();
}

/**
 * @brief TextureRenderThread::nodeAdded
 * @param newNode
 *
 * Adds a node to the internal node list.
 */
void TextureRenderThread::nodeAdded(TextureNodePtr newNode)
{
   if (!nodes.contains(newNode->getId())) {
      nodes.insert(newNode->getId(), newNode);
      generate();
   }
}

/**
 * @brief TextureRenderThread::nodeRemoved
 * @param id Node id
 *
 * Removes a node from the internal node list, thus no longer
 * rendering images for that node.
 */
void TextureRenderThread::nodeRemoved(int id)
{
   if (nodes.contains(id)) {
      nodes.remove(id);
      generate();
   }
}
