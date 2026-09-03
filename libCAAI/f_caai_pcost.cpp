/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：工序产品成本计算，并对金额尾差进行处理）
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
BM2_FUNCTION_EXPORT
double f_caai_getprice(const CString& mat_code, const CString& price_terms, const CString& dept_code, const CString& stats_period, CDbConnection * conn);
int f_caai_unit(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_sb(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_result(CString stats_period, CString dept_code, CDbConnection * conn);
int f_cm_mesac1_snd(CString stats_period, CString dept_code, CDbConnection * conn);
int f_cm_mesac3_snd(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_pcost(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;
	CDecimal price_gj = 0;
	CDecimal price_cj = 0;
	CDecimal price_yj = 0;
	CDecimal all_wt = 0;
	CString shift_no = "";
	CString shift_group = "";

	CString sqlstr = "";
	CString sqlstr1 = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);
	CDbCommand cmd_inq_a(conn);

	
	CModel tcaai03("TCAAI03");
	CModel tcaac10("TCAAC10");

	try
	{
		//产出
		sqlstr = " delete from tcaai03"
			" where 1=1"
			" AND DEPT_CODE = @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " delete from tcaai04"
			" where 1=1"
			" AND DEPT_CODE= @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = "insert into tcaai03(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,QTY,WT"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 )"
			" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,SUM(QTY),SUM(WT)"
			", prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			" FROM TCAAIS1"
			" WHERE 1=1"
			" AND DEPT_CODE= @dept_code"
			" AND PRO_FLAG ='O' " //产出工序
			" AND stats_period = @stats_period"
			" group by  DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE"
			", prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		//投入
		sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,MAT_CODE,WT,COST_GJ,COST_YJ,COST_CJ"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 )"
			" SELECT @datenow,DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,MAT_CODE,SUM(WT),SUM(AMT),SUM(AMT),SUM(AMT)"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			" FROM TCAAI02"
			" WHERE 1=1"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			" GROUP BY DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,MAT_CODE"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		//如果单日无产出，则消耗也赋值到投入中去
		sqlstr = " select sub_Backlog_code"
			" from tcaac14"
			" where code_line = @dept_code"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			sqlstr = "select SUM(WT)"
				" FROM TCAAIS1"
				" WHERE 1=1"
				" AND sub_Backlog_code= @sub_backlog_code"
				" AND PRO_FLAG ='O' " //产出工序
				" AND stats_period = @stats_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
			cmd_inq_1.ExecuteReader();
			all_wt = 0;
			if (cmd_inq_1.Read())
			{
				all_wt = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();
			if (all_wt == 0)
			{
				//实绩消耗量
				sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,WT"
					" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,data_from )"
					" SELECT @datenow,DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,SUM(WT)"
					" , STATS_PERIOD, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,'wc'"
					" FROM TCAAIS1"
					" WHERE 1=1"
					" AND sub_backlog_code = @sub_backlog_code"
					" AND stats_period = @stats_period"
					" GROUP BY DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE"
					" , prod_shift_group, prod_shift_no, sub_backlog_code, equ_no "
					" HAVING SUM(WT)!=0"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

				//分摊量
				sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,WT,COST_GJ,COST_YJ,COST_CJ"
					" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,data_from )"
					" SELECT @datenow,DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,SUM(WT),SUM(AMT),SUM(AMT),SUM(AMT)"
					" , STATS_PERIOD, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,'wc'"
					" FROM TCAAIS2"
					" WHERE 1=1"
					" AND sub_backlog_code = @sub_backlog_code"
					" AND stats_period = @stats_period"
					" GROUP BY DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE"
					" , prod_shift_group, prod_shift_no, sub_backlog_code, equ_no "
					" HAVING SUM(WT)!=0 OR SUM(AMT)!=0"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else
			{//班组产量为空
				sqlstr = " select distinct prod_shift_group "
					" FROM TCAAIS1"
					" WHERE 1=1"
					" AND sub_Backlog_code= @sub_backlog_code"
					" AND PRO_FLAG ='I' " //产出工序
					" and prod_shift_group!=' '"
					" AND stats_period = @stats_period"
					;
				cmd_inq_sub.SetCommandText(sqlstr);
				cmd_inq_sub.Parameters.Set("stats_period", stats_period);
				cmd_inq_sub.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_sub.ExecuteReader();
				while (cmd_inq_sub.Read())
				{
					sqlstr = "select SUM(WT)"
						" FROM TCAAIS1"
						" WHERE 1=1"
						" and RELATION_NO in (select RELATION_NO from tcaais1 WHERE  prod_shift_group=@prod_shift_group AND sub_Backlog_code= @sub_backlog_code  AND stats_period = @stats_period)"
						" AND sub_Backlog_code= @sub_backlog_code"
						" AND PRO_FLAG ='O' " //产出工序
						" AND stats_period = @stats_period"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("prod_shift_group", cmd_inq_sub.GetString(1));
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteReader();
					all_wt = 0;
					if (cmd_inq_1.Read())
					{
						all_wt = cmd_inq_1.GetDecimal(1);
					}
					cmd_inq_1.Close();
					if (all_wt == 0)
					{
						//实绩消耗量
						sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,wt"
							" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,data_from )"
							" SELECT @datenow,DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,SUM(WT)"
							" , STATS_PERIOD, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,'wc'"
							" FROM TCAAIS1"
							" WHERE 1=1"
							" and prod_shift_group=@prod_shift_group"
							" AND sub_backlog_code = @sub_backlog_code"
							" AND stats_period = @stats_period"
							" GROUP BY DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE"
							" , prod_shift_group, prod_shift_no, sub_backlog_code, equ_no "
							" HAVING SUM(WT)!=0"
							;
						cmd_inq_1.SetCommandText(sqlstr);
						cmd_inq_1.Parameters.Set("datenow", datenow);
						cmd_inq_1.Parameters.Set("prod_shift_group", cmd_inq_sub.GetString(1));
						cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
						cmd_inq_1.Parameters.Set("stats_period", stats_period);
						cmd_inq_1.ExecuteNonQuery();
						cmd_inq_1.Close();

						//分摊量
						sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,WT,COST_GJ,COST_YJ,COST_CJ"
							" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,data_from )"
							" SELECT @datenow,DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,SUM(WT),SUM(AMT),SUM(AMT),SUM(AMT)"
							" , STATS_PERIOD, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,'wc'"
							" FROM TCAAIS2"
							" WHERE 1=1"
							" and prod_shift_group=@prod_shift_group"
							" AND sub_backlog_code = @sub_backlog_code"
							" AND stats_period = @stats_period"
							" GROUP BY DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE"
							" , prod_shift_group, prod_shift_no, sub_backlog_code, equ_no "
							" HAVING SUM(WT)!=0 OR SUM(AMT)!=0"
							;
						cmd_inq_1.SetCommandText(sqlstr);
						cmd_inq_1.Parameters.Set("datenow", datenow);
						cmd_inq_1.Parameters.Set("prod_shift_group", cmd_inq_sub.GetString(1));
						cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
						cmd_inq_1.Parameters.Set("stats_period", stats_period);
						cmd_inq_1.ExecuteNonQuery();
						cmd_inq_1.Close();
					}
				}
				cmd_inq_sub.Close();
			}

			//能源
			sqlstr = " select distinct shift_no "
				" FROM TCAAIA12"
				" WHERE 1=1"
				" AND sub_Backlog_code= @sub_backlog_code"
				" and shift_no!=' '"
				" AND prod_date = @stats_period"
				;
			cmd_inq_sub.SetCommandText(sqlstr);
			cmd_inq_sub.Parameters.Set("stats_period", stats_period);
			cmd_inq_sub.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
			cmd_inq_sub.ExecuteReader();
			while (cmd_inq_sub.Read())
			{
				sqlstr = "select SUM(WT)"
					" FROM TCAAIS1"
					" WHERE 1=1"
					" and prod_shift_no=@prod_shift_no"
					" AND sub_Backlog_code= @sub_backlog_code"
					" AND PRO_FLAG ='O' " //产出工序
					" AND stats_period = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("prod_shift_no", cmd_inq_sub.GetString(1));
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.ExecuteReader();
				all_wt = 0;
				if (cmd_inq_1.Read())
				{
					all_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();
				if (all_wt == 0)
				{
					//能源
					sqlstr = "insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER,MAT_CODE,WT"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no,data_from)"
						" SELECT @datenow,DEPT_CODE, PROD_DATE, sub_backlog_code,MAT_CODE,SUM(COMSUME_WT)"
						" , PROD_DATE, shift_group, shift_no, sub_backlog_code, equ_no,'wcny'"
						" FROM TCAAIA12"
						" WHERE 1=1"
						" and shift_no =@shift_no"
						" AND sub_backlog_code = @sub_backlog_code"
						" AND PROD_DATE = @stats_period"
						" GROUP BY DEPT_CODE, sub_backlog_code,MAT_CODE"
						" , prod_date, shift_group, shift_no, sub_backlog_code, equ_no "
						" HAVING SUM(COMSUME_WT)!=0 "
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.Parameters.Set("shift_no", cmd_inq_sub.GetString(1));
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();					
				}
			}
			cmd_inq_sub.Close();

			sqlstr = " update tcaai04 t1 set product_code = (select CODE_DESC_1_CONTENT from TEP0002 t2 where t1.sub_Backlog_code = t2.code and t2.CODE_CLASS = 'CA29' )"
				" where 1=1"
				" and exists (select 1 from TEP0002 t2 where t1.sub_Backlog_code = t2.code and t2.CODE_CLASS = 'CA29')"
				" and data_from in ('wc','wcny')"
				" AND sub_backlog_code = @sub_backlog_code"
				" and dept_code=@dept_code "
				" AND sub_backlog_code = @sub_backlog_code"
				" and stats_period=@stats_period "
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

		}
		cmd_inq.Close();



		/*doFlag = f_caai_unit(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			strcpy(s.msg, "吨钢消耗失败!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		doFlag = f_caai_sb(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
		strcpy(s.msg, "包装消耗失败!");
		throw CApplicationException(-1, s.msg, log.Location);
		}


		doFlag = f_caai_result(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
		strcpy(s.msg, "最后一天将有投无产平摊本月!");
		throw CApplicationException(-1, s.msg, log.Location);
		}*/



		sqlstr = "select sub_Backlog_code"
			" from tcaac14"
			" where code_line = @dept_code"
			" order by SCHE_NO"
			;
		cmd_inq_a.SetCommandText(sqlstr);
		cmd_inq_a.Parameters.Set("dept_code", dept_code);
		cmd_inq_a.ExecuteReader();
		while (cmd_inq_a.Read())
		{
			//更新采购价
			sqlstr = " select distinct mat_code from tcaai04 "
				" WHERE 1=1"
				" and WT<>0 "
				" and sub_Backlog_code = @sub_backlog_code"
				" AND dept_code = @dept_code"
				" AND stats_period = @stats_period";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				price_gj = f_caai_getprice(cmd_inq.GetString(1), "GJ", dept_code, stats_period, conn);
				if (cmd_inq.GetString(1).GetLength() > 19) //如果坯料，取统一的价格，分不同的工序
				{
					price_yj = f_caai_getprice("PL_"+ cmd_inq_a.GetString(1), "YJ", dept_code, stats_period, conn);
				}
				else
				{
					price_yj = f_caai_getprice(cmd_inq.GetString(1), "YJ", dept_code, stats_period, conn);
				}
				price_cj = f_caai_getprice(cmd_inq.GetString(1), "CJ", dept_code, stats_period, conn);

				sqlstr = " update tcaai04 set price_unit_gj=@price_gj,cost_gj=round(wt*@price_gj,6)"
					",price_unit_yj=@price_yj,cost_yj=round(wt*@price_yj,6),price_unit_cj=@price_cj,cost_cj=round(wt*@price_cj,6) "
					" WHERE 1=1"
					" and mat_code=@mat_code"
					" and wt<>0 "
					" and sub_Backlog_code = @sub_backlog_code"
					" AND dept_code= @dept_code"
					" AND stats_period = @stats_period";
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("price_gj", price_gj);
				cmd_inq_1.Parameters.Set("price_yj", price_yj);
				cmd_inq_1.Parameters.Set("price_cj", price_cj);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

			}
			cmd_inq.Close();

			//更新产量 
			//因为关联数据项太多无法直接一条sql执行，改为分类执行
			sqlstr = " select DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,QTY,WT"
				" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
				" from tcaai03"
				" WHERE 1=1"
				" and sub_Backlog_code = @sub_backlog_code"
				" AND dept_code = @dept_code"
				" AND stats_period = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tcaai03);
				sqlstr = " update tcaai04 t1 set output = @WT"
					" where 1=1"
					" and t1.COST_CENTER =@COST_CENTER"
					" and t1.PRODUCT_CODE =@PRODUCT_CODE  "
					" and t1.vfree1=@VFREE1"
					" and t1.vfree2=@VFREE2"
					" and t1.vfree3=@VFREE3"
					" and t1.vfree4=@VFREE4"
					" and t1.vfree5=@VFREE5"
					" and t1.prod_date=@PROD_DATE"
					" and t1.prod_shift_group=@PROD_SHIFT_GROUP"
					" and t1.prod_shift_no=@PROD_SHIFT_NO"
					" and t1.mat_thick=@MAT_THICK"
					" and t1.mat_width=@MAT_WIDTH"
					" and t1.equ_no=@EQU_NO"
					" and t1.sg_sign=@SG_SIGN"
					" and t1.sub_backlog_code=@SUB_BACKLOG_CODE "
					" AND dept_code = @dept_code"
					" AND stats_period = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("WT", tcaai03["WT"].ToDecimal());
				cmd_inq_1.Parameters.Set("COST_CENTER", tcaai03["COST_CENTER"].ToString());
				cmd_inq_1.Parameters.Set("PRODUCT_CODE", tcaai03["PRODUCT_CODE"].ToString());
				cmd_inq_1.Parameters.Set("VFREE1", tcaai03["VFREE1"].ToString());
				cmd_inq_1.Parameters.Set("VFREE2", tcaai03["VFREE2"].ToString());
				cmd_inq_1.Parameters.Set("VFREE3", tcaai03["VFREE3"].ToString());
				cmd_inq_1.Parameters.Set("VFREE4", tcaai03["VFREE4"].ToString());
				cmd_inq_1.Parameters.Set("VFREE5", tcaai03["VFREE5"].ToString());
				cmd_inq_1.Parameters.Set("PROD_DATE", tcaai03["PROD_DATE"].ToString());
				cmd_inq_1.Parameters.Set("PROD_SHIFT_GROUP", tcaai03["PROD_SHIFT_GROUP"].ToString());
				cmd_inq_1.Parameters.Set("PROD_SHIFT_NO", tcaai03["PROD_SHIFT_NO"].ToString());
				cmd_inq_1.Parameters.Set("MAT_THICK", tcaai03["MAT_THICK"].ToDecimal());
				cmd_inq_1.Parameters.Set("MAT_WIDTH", tcaai03["MAT_WIDTH"].ToDecimal());
				cmd_inq_1.Parameters.Set("EQU_NO", tcaai03["EQU_NO"].ToString());
				cmd_inq_1.Parameters.Set("SG_SIGN", tcaai03["SG_SIGN"].ToString());
				cmd_inq_1.Parameters.Set("SUB_BACKLOG_CODE", tcaai03["SUB_BACKLOG_CODE"].ToString());
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();


				//更新金额
				sqlstr = " update tcaai03 t1 set (cost_gj,cost_yj,cost_cj) = (select sum(cost_gj),sum(cost_yj),sum(cost_cj) from tcaai04 where 1=1 "
					" and COST_CENTER =@COST_CENTER and PRODUCT_CODE =@PRODUCT_CODE  "
					" and vfree1=@VFREE1 and vfree2=@VFREE2 and vfree3=@VFREE3 and vfree4=@VFREE4 and vfree5=@VFREE5"
					" and prod_date=@PROD_DATE and prod_shift_group=@PROD_SHIFT_GROUP  and prod_shift_no=@PROD_SHIFT_NO  and mat_thick=@MAT_THICK and mat_width=@MAT_WIDTH  and equ_no=@EQU_NO and sg_sign=@SG_SIGN and sub_backlog_code=@SUB_BACKLOG_CODE "
					")"
					" where 1=1"
					" and exists (select 1 from tcaai04 where 1=1 "
					" and COST_CENTER =@COST_CENTER and PRODUCT_CODE =@PRODUCT_CODE  "
					" and vfree1=@VFREE1 and vfree2=@VFREE2 and vfree3=@VFREE3 and vfree4=@VFREE4 and vfree5=@VFREE5"
					" and prod_date=@PROD_DATE and prod_shift_group=@PROD_SHIFT_GROUP  and prod_shift_no=@PROD_SHIFT_NO  and mat_thick=@MAT_THICK and mat_width=@MAT_WIDTH  and equ_no=@EQU_NO and sg_sign=@SG_SIGN and sub_backlog_code=@SUB_BACKLOG_CODE "
					" )"
					" and t1.COST_CENTER =@COST_CENTER and t1.PRODUCT_CODE =@PRODUCT_CODE  "
					" and t1.vfree1=@VFREE1 and t1.vfree2=@VFREE2 and t1.vfree3=@VFREE3 and t1.vfree4=@VFREE4 and t1.vfree5=@VFREE5"
					" and t1.prod_date=@PROD_DATE and t1.prod_shift_group=@PROD_SHIFT_GROUP  and t1.prod_shift_no=@PROD_SHIFT_NO  and t1.mat_thick=@MAT_THICK and t1.mat_width=@MAT_WIDTH  and t1.equ_no=@EQU_NO and t1.sg_sign=@SG_SIGN and t1.sub_backlog_code=@SUB_BACKLOG_CODE "
					" AND dept_code = @dept_code"
					" AND stats_period = @stats_period"
					;
				//Log::Trace("", "", "sqlstr = [{0}][{1}][{2}][{3}][{4}]", sqlstr, dept_code, stats_period, tcaai03["SUB_BACKLOG_CODE, tcaai03["PROD_DATE);
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("WT", tcaai03["WT"].ToDecimal());
				cmd_inq_1.Parameters.Set("COST_CENTER", tcaai03["COST_CENTER"].ToString());
				cmd_inq_1.Parameters.Set("PRODUCT_CODE", tcaai03["PRODUCT_CODE"].ToString());
				cmd_inq_1.Parameters.Set("VFREE1", tcaai03["VFREE1"].ToString());
				cmd_inq_1.Parameters.Set("VFREE2", tcaai03["VFREE2"].ToString());
				cmd_inq_1.Parameters.Set("VFREE3", tcaai03["VFREE3"].ToString());
				cmd_inq_1.Parameters.Set("VFREE4", tcaai03["VFREE4"].ToString());
				cmd_inq_1.Parameters.Set("VFREE5", tcaai03["VFREE5"].ToString());
				cmd_inq_1.Parameters.Set("PROD_DATE", tcaai03["PROD_DATE"].ToString());
				cmd_inq_1.Parameters.Set("PROD_SHIFT_GROUP", tcaai03["PROD_SHIFT_GROUP"].ToString());
				cmd_inq_1.Parameters.Set("PROD_SHIFT_NO", tcaai03["PROD_SHIFT_NO"].ToString());
				cmd_inq_1.Parameters.Set("MAT_THICK", tcaai03["MAT_THICK"].ToDecimal());
				cmd_inq_1.Parameters.Set("MAT_WIDTH", tcaai03["MAT_WIDTH"].ToDecimal());
				cmd_inq_1.Parameters.Set("EQU_NO", tcaai03["EQU_NO"].ToString());
				cmd_inq_1.Parameters.Set("SG_SIGN", tcaai03["SG_SIGN"].ToString());
				cmd_inq_1.Parameters.Set("SUB_BACKLOG_CODE", tcaai03["SUB_BACKLOG_CODE"].ToString());
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			cmd_inq.Close();

			////将产品实际价格插入到价格表，作为新工序的消耗表
			//if ((dept_code == "IR" || cmd_inq_a.GetString(1) == "IS") && cmd_inq_a.GetString(1) != "TRT")
			//{
			//	sqlstr = " delete from tcaac12 "
			//		" where dept_code in ('IR','S1')"
			//		" and price_terms = 'CJ'"
			//		"  and valid_time_start = @stats_period"
			//		" AND (MAT_CODE IN (SELECT product_code FROM  tcaai03  where sub_Backlog_code = @sub_backlog_code AND dept_code = @dept_code AND stats_period = @stats_period) or (@sub_backlog_code = 'BF' AND mat_code in ('0106010001')))"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//	sqlstr = " insert into tcaac12 ( rec_creator, rec_create_time, price_terms, DEPT_CODE,mat_code,mat_name,price_unit, valid_time_start,valid_flag,back_code_1)"
			//		" select @rec_creator, @rec_create_time,'CJ','IR',t.product_code,t2.mat_name,round(sum(cost_cj)/SUM(wt),6),@stats_period,'1','mm'  "
			//		" from tcaai03 t"
			//		" left join tcaac11 t2 on t.product_code= t2.mat_code"
			//		" where t.sub_Backlog_code = @sub_backlog_code"
			//		" AND t.dept_code = @dept_code"
			//		" AND t.stats_period = @stats_period"
			//		" group by t.product_code,t2.mat_name"
			//		" having round(sum(cost_cj)/SUM(wt),6)>0"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//	sqlstr = " insert into tcaac12 ( rec_creator, rec_create_time, price_terms, DEPT_CODE,mat_code,mat_name,price_unit, valid_time_start,valid_flag,back_code_1)"
			//		" select @rec_creator, @rec_create_time,'CJ','S1',t.product_code,mat_name,round(sum(cost_cj)/SUM(wt),6),@stats_period,'1','mm'  "
			//		" from tcaai03 t"
			//		" left join tcaac11 t2 on t.product_code= t2.mat_code"
			//		" where t.sub_Backlog_code = @sub_backlog_code"
			//		" AND t.dept_code = @dept_code"
			//		" AND t.stats_period = @stats_period"
			//		" group by t.product_code,t2.mat_name"
			//		" having round(sum(cost_cj)/SUM(wt),6)>0"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//	//生铁为铁水+23块
			//	sqlstr = " insert into tcaac12 ( rec_creator, rec_create_time, price_terms, DEPT_CODE,mat_code,mat_name,price_unit, valid_time_start,valid_flag,back_code_1)"
			//		" select @rec_creator, @rec_create_time,'CJ','S1','0106010001','生铁',round(sum(cost_cj)/SUM(wt),6)+23,@stats_period,'1','mm'  "
			//		" from tcaai03 t"
			//		//" left join tcaac11 t2 on t.product_code= t2.mat_code"
			//		" where t.sub_Backlog_code = @sub_backlog_code"
			//		" and product_code in （'01090005')"
			//		" AND t.dept_code = @dept_code"
			//		" AND t.stats_period = @stats_period"
			//		"  HAVING SUM(wt)>0"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();
			//}

			//if (dept_code == "S1" &&  cmd_inq_a.GetString(1) == "C")
			//{
			//	sqlstr = " delete from tcaac12 "
			//		" where 1=1"
			//		" and price_terms = 'CJ'"
			//		"  and valid_time_start = @stats_period"
			//		" AND (MAT_CODE LIKE 'rcb0014_%' or mat_code like 'rcb0015_%')"
			//		" and dept_code in ('D','B')"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//	sqlstr = " insert into tcaac12 ( rec_creator, rec_create_time, price_terms, DEPT_CODE,mat_code,mat_name,price_unit, valid_time_start,valid_flag,back_code_1)"
			//		" select @rec_creator, @rec_create_time,'CJ','D','rcb0015_'||sg_sign,'线材钢锭',round(sum(cost_cj)/SUM(decode(sub_backlog_code,'C',wt,0)),6),@stats_period,'1','mm'  "
			//		" from tcaai03 t"
			//		" where 1=1"
			//		" AND t.dept_code = @dept_code"
			//		" AND t.stats_period = @stats_period"
			//		" group by t.sg_sign"
			//		" HAVING SUM(decode(sub_backlog_code,'C',wt,0))!=0 AND  round(sum(cost_cj)/SUM(decode(sub_backlog_code,'C',wt,0)),6)>1000"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//	sqlstr = " insert into tcaac12 ( rec_creator, rec_create_time, price_terms, DEPT_CODE,mat_code,mat_name,price_unit, valid_time_start,valid_flag,back_code_1)"
			//		" select @rec_creator, @rec_create_time,'CJ','B','rcb0014_'||sg_sign,'rcb0014',round(sum(cost_cj)/SUM(decode(sub_backlog_code,'C',wt,0)),6),@stats_period,'1','mm'  "
			//		" from tcaai03 t"
			//		" where 1=1"
			//		" AND t.dept_code = @dept_code"
			//		" AND t.stats_period = @stats_period"
			//		" group by t.sg_sign"
			//		" HAVING SUM(decode(sub_backlog_code,'C',wt,0))!=0 AND  round(sum(cost_cj)/SUM(decode(sub_backlog_code,'C',wt,0)),6)>1000"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
			//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
			//	cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_a.GetString(1));
			//	cmd_inq_1.Parameters.Set("rec_creator", s.userid);
			//	cmd_inq_1.Parameters.Set("rec_create_time", datenow);
			//	cmd_inq_1.ExecuteNonQuery();
			//	cmd_inq_1.Close();

			//}
		}
		cmd_inq_a.Close();


		sqlstr = " update tcaai03 set PRICE_UNIT = round(cost_GJ/WT,6),PRICE_UNIT_GJ = round(cost_GJ/WT,6),PRICE_UNIT_YJ = round(cost_YJ/WT,6),PRICE_UNIT_CJ = round(cost_CJ/WT,6)"
			" where 1=1"
			" and wt!=0"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		

		//更新物料名称
		sqlstr = "update tcaai04 t1 set mat_name = (select mat_name from tcaac11 t2 where decode (INSTR(t1.mat_code,'_', 1, 1),0,t1.mat_code,substr(t1.mat_code,1,INSTR(t1.mat_code,'_', 1, 1)-1)) = t2.mat_code)"
			" where 1=1"
			" and exists (select 1 from tcaac11 t2 where decode (INSTR(t1.mat_code,'_', 1, 1),0,t1.mat_code,substr(t1.mat_code,1,INSTR(t1.mat_code,'_', 1, 1)-1)) = t2.mat_code)"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
