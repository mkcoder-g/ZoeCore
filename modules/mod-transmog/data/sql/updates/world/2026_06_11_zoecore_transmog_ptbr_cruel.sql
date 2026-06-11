-- ZoeCore Transmog V1 PT-BR / Cruel
-- Aplicar no WORLD depois dos SQLs originais do módulo.

-- NPC principal
UPDATE `creature_template`
SET `name` = 'ZoeCore Transmog',
    `subname` = 'Visual Master',
    `npcflag` = (`npcflag` | 1)
WHERE `entry` IN (190010,190011);

DELETE FROM `creature_template_locale` WHERE `entry` IN (190010,190011) AND `locale` = 'ptBR';
INSERT INTO `creature_template_locale` (`entry`, `locale`, `Name`, `Title`) VALUES
(190010, 'ptBR', 'ZoeCore Transmog', 'Mestre Visual'),
(190011, 'ptBR', 'ZoeCore Transmog Portátil', 'Mestre Visual');

-- Itens utilitários
UPDATE `item_template`
SET `name` = 'Ocultar Visual Equipado',
    `description` = 'Oculta o visual deste slot pelo sistema de transmog.'
WHERE `entry` = 57575;

UPDATE `item_template`
SET `name` = 'Remover Transmog',
    `description` = 'Remove o transmog ativo deste item.'
WHERE `entry` = 57576;

DELETE FROM `item_template_locale` WHERE `ID` IN (57575,57576) AND `locale` = 'ptBR';
INSERT INTO `item_template_locale` (`ID`, `locale`, `Name`, `Description`) VALUES
(57575, 'ptBR', 'Ocultar Visual Equipado', 'Oculta o visual deste slot pelo sistema de transmog.'),
(57576, 'ptBR', 'Remover Transmog', 'Remove o transmog ativo deste item.');

-- Textos informativos
UPDATE `npc_text`
SET `text0_0` = 'Transmog permite mudar a aparência dos seus itens sem alterar atributos.

No ZoeCore, o transmog usa Fragmento Cruel como moeda padrão.
O item mantém seus status, sockets, enchants e bônus de set; apenas o visual muda.

Regras principais:
- Apenas armas e armaduras podem ser usadas.
- O visual precisa ser compatível com sua classe e tipo de item, conforme configuração.
- Você pode ocultar slots visuais quando permitido.
- Você pode salvar conjuntos de transmog.

Dica:
Use Ctrl + clique no item para pré-visualizar o visual.'
WHERE `ID` = 601083;

UPDATE `npc_text`
SET `text0_0` = 'Conjuntos de transmog permitem salvar uma combinação visual completa.

Como usar:
1. Aplique transmog nos itens equipados.
2. Abra Gerenciar conjuntos.
3. Salve o conjunto com um nome.
4. Depois você pode aplicar o conjunto salvo rapidamente.

Atenção:
Usar um conjunto pode substituir transmogs antigos nos slots usados.'
WHERE `ID` = 601084;

DELETE FROM `npc_text_locale` WHERE `ID` IN (601083,601084) AND `Locale` = 'ptBR';
INSERT INTO `npc_text_locale` (`ID`, `Locale`, `Text0_0`) VALUES
(601083, 'ptBR', 'Transmog permite mudar a aparência dos seus itens sem alterar atributos.

No ZoeCore, o transmog usa Fragmento Cruel como moeda padrão.
O item mantém seus status, sockets, enchants e bônus de set; apenas o visual muda.

Regras principais:
- Apenas armas e armaduras podem ser usadas.
- O visual precisa ser compatível com sua classe e tipo de item, conforme configuração.
- Você pode ocultar slots visuais quando permitido.
- Você pode salvar conjuntos de transmog.

Dica:
Use Ctrl + clique no item para pré-visualizar o visual.'),
(601084, 'ptBR', 'Conjuntos de transmog permitem salvar uma combinação visual completa.

Como usar:
1. Aplique transmog nos itens equipados.
2. Abra Gerenciar conjuntos.
3. Salve o conjunto com um nome.
4. Depois você pode aplicar o conjunto salvo rapidamente.

Atenção:
Usar um conjunto pode substituir transmogs antigos nos slots usados.');

-- Strings do módulo em PT-BR
UPDATE `module_string` SET `string` = 'Item transmogrifado com sucesso.' WHERE `module` = 'mod-transmog' AND `id` = 1;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 1 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',1,'ptBR','Item transmogrifado com sucesso.');
UPDATE `module_string` SET `string` = 'O slot de equipamento está vazio.' WHERE `module` = 'mod-transmog' AND `id` = 2;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 2 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',2,'ptBR','O slot de equipamento está vazio.');
UPDATE `module_string` SET `string` = 'Item visual selecionado inválido.' WHERE `module` = 'mod-transmog' AND `id` = 3;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 3 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',3,'ptBR','Item visual selecionado inválido.');
UPDATE `module_string` SET `string` = 'O item visual não existe.' WHERE `module` = 'mod-transmog' AND `id` = 4;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 4 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',4,'ptBR','O item visual não existe.');
UPDATE `module_string` SET `string` = 'O item de destino não existe.' WHERE `module` = 'mod-transmog' AND `id` = 5;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 5 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',5,'ptBR','O item de destino não existe.');
UPDATE `module_string` SET `string` = 'Os itens selecionados são inválidos.' WHERE `module` = 'mod-transmog' AND `id` = 6;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 6 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',6,'ptBR','Os itens selecionados são inválidos.');
UPDATE `module_string` SET `string` = 'Você não tem dinheiro suficiente.' WHERE `module` = 'mod-transmog' AND `id` = 7;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 7 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',7,'ptBR','Você não tem dinheiro suficiente.');
UPDATE `module_string` SET `string` = 'Você não tem Fragmento Cruel suficiente.' WHERE `module` = 'mod-transmog' AND `id` = 8;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 8 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',8,'ptBR','Você não tem Fragmento Cruel suficiente.');
UPDATE `module_string` SET `string` = 'Todos os seus transmogs foram removidos.' WHERE `module` = 'mod-transmog' AND `id` = 9;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 9 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',9,'ptBR','Todos os seus transmogs foram removidos.');
UPDATE `module_string` SET `string` = 'Nenhum transmog encontrado.' WHERE `module` = 'mod-transmog' AND `id` = 10;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 10 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',10,'ptBR','Nenhum transmog encontrado.');
UPDATE `module_string` SET `string` = 'Nome inválido.' WHERE `module` = 'mod-transmog' AND `id` = 11;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 11 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',11,'ptBR','Nome inválido.');
UPDATE `module_string` SET `string` = 'Exibindo itens transmogrifados. Relogue para atualizar a área atual.' WHERE `module` = 'mod-transmog' AND `id` = 12;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 12 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',12,'ptBR','Exibindo itens transmogrifados. Relogue para atualizar a área atual.');
UPDATE `module_string` SET `string` = 'Ocultando itens transmogrifados. Relogue para atualizar a área atual.' WHERE `module` = 'mod-transmog' AND `id` = 13;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 13 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',13,'ptBR','Ocultando itens transmogrifados. Relogue para atualizar a área atual.');
UPDATE `module_string` SET `string` = 'O item selecionado não é adequado para transmog.' WHERE `module` = 'mod-transmog' AND `id` = 14;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 14 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',14,'ptBR','O item selecionado não é adequado para transmog.');
UPDATE `module_string` SET `string` = 'O item selecionado não pode ser usado como visual para esse jogador.' WHERE `module` = 'mod-transmog' AND `id` = 15;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 15 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',15,'ptBR','O item selecionado não pode ser usado como visual para esse jogador.');
UPDATE `module_string` SET `string` = 'Sincronizando coleção de aparências...' WHERE `module` = 'mod-transmog' AND `id` = 16;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 16 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',16,'ptBR','Sincronizando coleção de aparências...');
UPDATE `module_string` SET `string` = 'Sincronização de aparências concluída.' WHERE `module` = 'mod-transmog' AND `id` = 17;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 17 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',17,'ptBR','Sincronização de aparências concluída.');
UPDATE `module_string` SET `string` = 'O NPC de transmog agora exibirá aparências disponíveis em interface de vendedor, permitindo pré-visualização.\\nAVISO: se você tiver muitas aparências, algumas podem não aparecer por limitação do client. Nesse caso, desative esta opção.' WHERE `module` = 'mod-transmog' AND `id` = 18;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 18 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',18,'ptBR','O NPC de transmog agora exibirá aparências disponíveis em interface de vendedor, permitindo pré-visualização.\\nAVISO: se você tiver muitas aparências, algumas podem não aparecer por limitação do client. Nesse caso, desative esta opção.');
UPDATE `module_string` SET `string` = 'O NPC de transmog agora exibirá aparências via lista de gossip.' WHERE `module` = 'mod-transmog' AND `id` = 19;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 19 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',19,'ptBR','O NPC de transmog agora exibirá aparências via lista de gossip.');
UPDATE `module_string` SET `string` = 'Como funciona o transmog?' WHERE `module` = 'mod-transmog' AND `id` = 20;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 20 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',20,'ptBR','Como funciona o transmog?');
UPDATE `module_string` SET `string` = 'Gerenciar conjuntos' WHERE `module` = 'mod-transmog' AND `id` = 21;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 21 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',21,'ptBR','Gerenciar conjuntos');
UPDATE `module_string` SET `string` = 'Remover todos os transmogs' WHERE `module` = 'mod-transmog' AND `id` = 22;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 22 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',22,'ptBR','Remover todos os transmogs');
UPDATE `module_string` SET `string` = 'Remover transmogs de todos os itens equipados?' WHERE `module` = 'mod-transmog' AND `id` = 23;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 23 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',23,'ptBR','Remover transmogs de todos os itens equipados?');
UPDATE `module_string` SET `string` = 'Atualizar menu' WHERE `module` = 'mod-transmog' AND `id` = 24;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 24 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',24,'ptBR','Atualizar menu');
UPDATE `module_string` SET `string` = 'Como funcionam os conjuntos?' WHERE `module` = 'mod-transmog' AND `id` = 25;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 25 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',25,'ptBR','Como funcionam os conjuntos?');
UPDATE `module_string` SET `string` = 'Salvar conjunto' WHERE `module` = 'mod-transmog' AND `id` = 26;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 26 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',26,'ptBR','Salvar conjunto');
UPDATE `module_string` SET `string` = 'Voltar...' WHERE `module` = 'mod-transmog' AND `id` = 27;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 27 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',27,'ptBR','Voltar...');
UPDATE `module_string` SET `string` = 'Usar este conjunto' WHERE `module` = 'mod-transmog' AND `id` = 28;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 28 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',28,'ptBR','Usar este conjunto');
UPDATE `module_string` SET `string` = 'Usar este conjunto para transmog irá vincular os itens transmogrifados a você e torná-los não reembolsáveis e não negociáveis.\\nDeseja continuar?\\n\\n' WHERE `module` = 'mod-transmog' AND `id` = 29;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 29 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',29,'ptBR','Usar este conjunto para transmog irá vincular os itens transmogrifados a você e torná-los não reembolsáveis e não negociáveis.\\nDeseja continuar?\\n\\n');
UPDATE `module_string` SET `string` = 'Excluir conjunto' WHERE `module` = 'mod-transmog' AND `id` = 30;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 30 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',30,'ptBR','Excluir conjunto');
UPDATE `module_string` SET `string` = 'Tem certeza que deseja excluir ' WHERE `module` = 'mod-transmog' AND `id` = 31;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 31 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',31,'ptBR','Tem certeza que deseja excluir ');
UPDATE `module_string` SET `string` = 'Digite o nome do conjunto' WHERE `module` = 'mod-transmog' AND `id` = 32;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 32 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',32,'ptBR','Digite o nome do conjunto');
UPDATE `module_string` SET `string` = 'Pesquisar...' WHERE `module` = 'mod-transmog' AND `id` = 33;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 33 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',33,'ptBR','Pesquisar...');
UPDATE `module_string` SET `string` = 'Pesquisando por: ' WHERE `module` = 'mod-transmog' AND `id` = 34;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 34 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',34,'ptBR','Pesquisando por: ');
UPDATE `module_string` SET `string` = 'Qual item deseja pesquisar?' WHERE `module` = 'mod-transmog' AND `id` = 35;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 35 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',35,'ptBR','Qual item deseja pesquisar?');
UPDATE `module_string` SET `string` = 'Você está ocultando o item deste slot.\\nDeseja continuar?\\n\\n' WHERE `module` = 'mod-transmog' AND `id` = 36;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 36 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',36,'ptBR','Você está ocultando o item deste slot.\\nDeseja continuar?\\n\\n');
UPDATE `module_string` SET `string` = 'Ocultar slot' WHERE `module` = 'mod-transmog' AND `id` = 37;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 37 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',37,'ptBR','Ocultar slot');
UPDATE `module_string` SET `string` = 'Remover transmog deste slot?' WHERE `module` = 'mod-transmog' AND `id` = 38;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 38 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',38,'ptBR','Remover transmog deste slot?');
UPDATE `module_string` SET `string` = 'Usar este item como visual irá vinculá-lo a você e torná-lo não reembolsável e não negociável.\\nDeseja continuar?\\n\\n' WHERE `module` = 'mod-transmog' AND `id` = 39;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 39 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',39,'ptBR','Usar este item como visual irá vinculá-lo a você e torná-lo não reembolsável e não negociável.\\nDeseja continuar?\\n\\n');
UPDATE `module_string` SET `string` = 'Página anterior' WHERE `module` = 'mod-transmog' AND `id` = 40;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 40 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',40,'ptBR','Página anterior');
UPDATE `module_string` SET `string` = 'Próxima página' WHERE `module` = 'mod-transmog' AND `id` = 41;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 41 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',41,'ptBR','Próxima página');
UPDATE `module_string` SET `string` = 'foi adicionado à sua coleção de aparências.' WHERE `module` = 'mod-transmog' AND `id` = 42;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 42 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',42,'ptBR','foi adicionado à sua coleção de aparências.');
UPDATE `module_string` SET `string` = '|cFF4DB3FFBônus de set não aparecem no tooltip enquanto o item está transmogrifado, mas continuam totalmente ativos.\\nPara ocultar este aviso, digite |cFFFFFFFF.transmog disclaimer off|cFF4DB3FF.|r' WHERE `module` = 'mod-transmog' AND `id` = 43;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 43 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',43,'ptBR','|cFF4DB3FFBônus de set não aparecem no tooltip enquanto o item está transmogrifado, mas continuam totalmente ativos.\\nPara ocultar este aviso, digite |cFFFFFFFF.transmog disclaimer off|cFF4DB3FF.|r');
UPDATE `module_string` SET `string` = 'Aviso de bônus de set ativado.' WHERE `module` = 'mod-transmog' AND `id` = 44;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 44 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',44,'ptBR','Aviso de bônus de set ativado.');
UPDATE `module_string` SET `string` = 'Aviso de bônus de set desativado.' WHERE `module` = 'mod-transmog' AND `id` = 45;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 45 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',45,'ptBR','Aviso de bônus de set desativado.');
UPDATE `module_string` SET `string` = '=== Diagnóstico de Transmog ===' WHERE `module` = 'mod-transmog' AND `id` = 46;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 46 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',46,'ptBR','=== Diagnóstico de Transmog ===');
UPDATE `module_string` SET `string` = 'Destino     : {}' WHERE `module` = 'mod-transmog' AND `id` = 47;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 47 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',47,'ptBR','Destino     : {}');
UPDATE `module_string` SET `string` = 'Visual      : {}' WHERE `module` = 'mod-transmog' AND `id` = 48;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 48 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',48,'ptBR','Visual      : {}');
UPDATE `module_string` SET `string` = 'Jogador     : {}' WHERE `module` = 'mod-transmog' AND `id` = 49;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 49 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',49,'ptBR','Jogador     : {}');
UPDATE `module_string` SET `string` = 'Jogador ''{}'' não encontrado.' WHERE `module` = 'mod-transmog' AND `id` = 50;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 50 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',50,'ptBR','Jogador ''{}'' não encontrado.');
UPDATE `module_string` SET `string` = '--- Verificações do par ---' WHERE `module` = 'mod-transmog' AND `id` = 51;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 51 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',51,'ptBR','--- Verificações do par ---');
UPDATE `module_string` SET `string` = '--- Item: {} ---' WHERE `module` = 'mod-transmog' AND `id` = 52;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 52 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',52,'ptBR','--- Item: {} ---');
UPDATE `module_string` SET `string` = '--- Coleção ---' WHERE `module` = 'mod-transmog' AND `id` = 53;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 53 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',53,'ptBR','--- Coleção ---');
UPDATE `module_string` SET `string` = '=== RESULTADO: PODE usar transmog ===' WHERE `module` = 'mod-transmog' AND `id` = 54;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 54 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',54,'ptBR','=== RESULTADO: PODE usar transmog ===');
UPDATE `module_string` SET `string` = '=== RESULTADO: NÃO pode usar transmog ===' WHERE `module` = 'mod-transmog' AND `id` = 55;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 55 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',55,'ptBR','=== RESULTADO: NÃO pode usar transmog ===');
UPDATE `module_string` SET `string` = 'Mesmo ID de item — não pode aplicar um item nele mesmo.' WHERE `module` = 'mod-transmog' AND `id` = 56;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 56 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',56,'ptBR','Mesmo ID de item — não pode aplicar um item nele mesmo.');
UPDATE `module_string` SET `string` = 'Mesmo DisplayID — o visual já é igual.' WHERE `module` = 'mod-transmog' AND `id` = 57;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 57 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',57,'ptBR','Mesmo DisplayID — o visual já é igual.');
UPDATE `module_string` SET `string` = 'Classe do item incompatível — ambos devem ser armadura ou ambos devem ser arma.' WHERE `module` = 'mod-transmog' AND `id` = 58;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 58 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',58,'ptBR','Classe do item incompatível — ambos devem ser armadura ou ambos devem ser arma.');
UPDATE `module_string` SET `string` = 'Tipo de inventário do destino proibido.' WHERE `module` = 'mod-transmog' AND `id` = 59;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 59 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',59,'ptBR','Tipo de inventário do destino proibido.');
UPDATE `module_string` SET `string` = 'Tipo de inventário do visual proibido.' WHERE `module` = 'mod-transmog' AND `id` = 60;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 60 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',60,'ptBR','Tipo de inventário do visual proibido.');
UPDATE `module_string` SET `string` = 'Tipo de arma de distância incompatível — não pode misturar ranged e melee.' WHERE `module` = 'mod-transmog' AND `id` = 61;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 61 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',61,'ptBR','Tipo de arma de distância incompatível — não pode misturar ranged e melee.');
UPDATE `module_string` SET `string` = 'Subclasse incompatível — não permitido.' WHERE `module` = 'mod-transmog' AND `id` = 62;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 62 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',62,'ptBR','Subclasse incompatível — não permitido.');
UPDATE `module_string` SET `string` = 'Tipo de inventário incompatível — não permitido.' WHERE `module` = 'mod-transmog' AND `id` = 63;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 63 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',63,'ptBR','Tipo de inventário incompatível — não permitido.');
UPDATE `module_string` SET `string` = 'Whitelist — todas as verificações foram ignoradas.' WHERE `module` = 'mod-transmog' AND `id` = 64;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 64 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',64,'ptBR','Whitelist — todas as verificações foram ignoradas.');
UPDATE `module_string` SET `string` = 'A classe do item deve ser armadura ou arma.' WHERE `module` = 'mod-transmog' AND `id` = 65;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 65 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',65,'ptBR','A classe do item deve ser armadura ou arma.');
UPDATE `module_string` SET `string` = 'Blacklist — não pode ser usado para transmog.' WHERE `module` = 'mod-transmog' AND `id` = 66;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 66 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',66,'ptBR','Blacklist — não pode ser usado para transmog.');
UPDATE `module_string` SET `string` = 'Qualidade não permitida pela configuração do servidor.' WHERE `module` = 'mod-transmog' AND `id` = 67;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 67 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',67,'ptBR','Qualidade não permitida pela configuração do servidor.');
UPDATE `module_string` SET `string` = 'Vara de pesca — não permitida.' WHERE `module` = 'mod-transmog' AND `id` = 68;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 68 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',68,'ptBR','Vara de pesca — não permitida.');
UPDATE `module_string` SET `string` = 'Evento/requisito sazonal não ativo.' WHERE `module` = 'mod-transmog' AND `id` = 69;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 69 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',69,'ptBR','Evento/requisito sazonal não ativo.');
UPDATE `module_string` SET `string` = 'Item sem atributos — não pode ser usado para transmog.' WHERE `module` = 'mod-transmog' AND `id` = 70;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 70 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',70,'ptBR','Item sem atributos — não pode ser usado para transmog.');
UPDATE `module_string` SET `string` = 'Proficiência ausente (skill: {}) — não permitido pela configuração.' WHERE `module` = 'mod-transmog' AND `id` = 71;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 71 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',71,'ptBR','Proficiência ausente (skill: {}) — não permitido pela configuração.');
UPDATE `module_string` SET `string` = 'Requisito de facção não cumprido.' WHERE `module` = 'mod-transmog' AND `id` = 72;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 72 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',72,'ptBR','Requisito de facção não cumprido.');
UPDATE `module_string` SET `string` = 'Requisito de classe não cumprido.' WHERE `module` = 'mod-transmog' AND `id` = 73;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 73 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',73,'ptBR','Requisito de classe não cumprido.');
UPDATE `module_string` SET `string` = 'Requisito de raça não cumprido.' WHERE `module` = 'mod-transmog' AND `id` = 74;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 74 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',74,'ptBR','Requisito de raça não cumprido.');
UPDATE `module_string` SET `string` = 'Skill necessária {} rank {} — não cumprida (atual: {}).' WHERE `module` = 'mod-transmog' AND `id` = 75;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 75 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',75,'ptBR','Skill necessária {} rank {} — não cumprida (atual: {}).');
UPDATE `module_string` SET `string` = 'Requisito de level não cumprido (necessário: {}, atual: {}).' WHERE `module` = 'mod-transmog' AND `id` = 76;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 76 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',76,'ptBR','Requisito de level não cumprido (necessário: {}, atual: {}).');
UPDATE `module_string` SET `string` = 'Spell necessária {} desconhecida.' WHERE `module` = 'mod-transmog' AND `id` = 77;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 77 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',77,'ptBR','Spell necessária {} desconhecida.');
UPDATE `module_string` SET `string` = 'Sistema de coleção desativado.' WHERE `module` = 'mod-transmog' AND `id` = 78;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 78 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',78,'ptBR','Sistema de coleção desativado.');
UPDATE `module_string` SET `string` = 'Aparência AUSENTE na coleção do jogador.' WHERE `module` = 'mod-transmog' AND `id` = 79;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 79 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',79,'ptBR','Aparência AUSENTE na coleção do jogador.');
UPDATE `module_string` SET `string` = 'Todas as verificações passaram.' WHERE `module` = 'mod-transmog' AND `id` = 80;
DELETE FROM `module_string_locale` WHERE `module` = 'mod-transmog' AND `id` = 80 AND `locale` = 'ptBR';
INSERT INTO `module_string_locale` (`module`,`id`,`locale`,`string`) VALUES ('mod-transmog',80,'ptBR','Todas as verificações passaram.');

-- Comando de diagnóstico PT-BR
UPDATE `command`
SET `help` = 'Sintaxe: .transmog check $player $destItem $srcItem\nExecuta diagnóstico completo para saber se $srcItem pode ser aplicado como visual em $destItem para $player.\nFunciona para players offline e console.'
WHERE `name` = 'transmog check';
