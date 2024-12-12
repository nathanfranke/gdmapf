### ⚠️ This will not receive updates, but I am still working on Godot pathfinding solutions. [Contact me](https://github.com/nathanfranke/)
### ⚠️ There is currently a major bug where **agents travelling toward each other next to lots of obstacles get stuck**. The solution would be to, instead of storing agent density at cells, storing agent velocity, and prioritize paths that travel along that velocity, but not against. However, the accuracy of this would be limited to the 8 directions that connect cells.

# gdmapf

GDExtension solution to the multi-agent pathfinding problem using a modified flow-field algorithm.

![Demonstration](https://raw.githubusercontent.com/nathanfranke/gdmapf/main/misc/demonstration.gif)
